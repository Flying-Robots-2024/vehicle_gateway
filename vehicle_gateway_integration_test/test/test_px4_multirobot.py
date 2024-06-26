#!/usr/bin/env python3
# Copyright 2023 Open Source Robotics Foundation, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import os
from shlex import split
import subprocess

import unittest
import xml.etree.ElementTree as ET

from ament_index_python.packages import get_package_share_directory

import launch
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription
from launch.actions import SetEnvironmentVariable
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration, PythonExpression
from launch_ros.actions import Node
import launch_testing
from launch_testing.actions import ReadyToTest
from launch_testing.util import KeepAliveProc

import psutil
import pytest

from vehicle_gateway_python_helpers.helpers import get_px4_dir
from vehicle_gateway_python_helpers.helpers import get_px4_process


def create_px4_instance(vehicle_id):
    gateway_models_dir = get_package_share_directory('vehicle_gateway_models')

    model_name = [LaunchConfiguration('vehicle_type'), '_stock_', vehicle_id]

    run_px4 = get_px4_process(vehicle_id, {'PX4_GZ_MODEL_NAME': model_name})

    model_sdf_filename = [
        gateway_models_dir,
        '/configs_px4/',
        LaunchConfiguration('vehicle_type'),
        '_',
        'stock',
        '/model.sdf']

    world_sdf_path = os.path.join(
        get_package_share_directory('vehicle_gateway_worlds'),
        'worlds',
        'empty_px4_world.sdf')
    # I couldn't get the libsdformat binding to work as expected due
    # to various troubles. Let's simplify and just treat SDF as
    # regular XML and do an XPath query
    sdf_root = ET.parse(world_sdf_path).getroot()
    frame_node = sdf_root.find(f'.//frame[@name=\"pad_{vehicle_id}\"]')
    if not frame_node:
        raise ValueError(f'Could not find a frame named pad_{vehicle_id}')
    pose_node = frame_node.find('pose')
    pose_str = pose_node.text
    x, y, z, roll, pitch, yaw = pose_str.split(' ')

    spawn_entity = Node(
        package='ros_gz_sim',
        executable='create',
        output='screen',
        arguments=['-file', model_sdf_filename,
                   '-name', model_name,
                   '-allow_renaming', 'true',
                   '-x', x,
                   '-y', y,
                   '-z', z,
                   '-R', roll,
                   '-P', pitch,
                   '-Y', yaw],
        on_exit=[run_px4])

    return [run_px4, spawn_entity]


# This function specifies the processes to be run for our test
@pytest.mark.launch_test
def generate_test_description():
    # This is necessary to get unbuffered output from the process under test
    proc_env = os.environ.copy()
    proc_env['PYTHONUNBUFFERED'] = '1'

    world_pkgs = get_package_share_directory('vehicle_gateway_worlds')
    gtw_models = get_package_share_directory('vehicle_gateway_models')

    os.environ['GZ_SIM_RESOURCE_PATH'] = ':' + os.path.join(get_px4_dir(), 'models')
    os.environ['GZ_SIM_RESOURCE_PATH'] += ':' + os.path.join(get_px4_dir(), 'worlds')
    os.environ['GZ_SIM_RESOURCE_PATH'] += ':' + os.path.join(world_pkgs, 'worlds')
    os.environ['GZ_SIM_RESOURCE_PATH'] += ':' + os.path.join(gtw_models, 'models')
    os.environ['GZ_SIM_RESOURCE_PATH'] += ':' + os.path.join(gtw_models, 'configs_px4')

    if 'SHOW_GZ_GUI' in os.environ and os.environ['SHOW_GZ_GUI']:
        gz_gui_args = ''
    else:
        gz_gui_args = '--headless-rendering -s'
    gz_args = f'{gz_gui_args} -r -v 4 empty_px4_world.sdf'

    included_launch = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            [os.path.join(get_package_share_directory('ros_gz_sim'),
                          'launch', 'gz_sim.launch.py')]),
        launch_arguments=[('gz_args', [gz_args])]
    )

    autostart_magic_number = PythonExpression([
        '{"x500": 4001, "rc_cessna": 4003, "standard_vtol": 4004}["',
        LaunchConfiguration('vehicle_type'),
        '"]'])

    autostart_env_var = SetEnvironmentVariable(
        'PX4_SYS_AUTOSTART',
        autostart_magic_number)

    micro_ros_agent = Node(
        package='micro_ros_agent',
        executable='micro_ros_agent',
        arguments=['udp4', '-p', '8888'],
        output='screen')

    ld = launch.LaunchDescription([
        DeclareLaunchArgument(
            'vehicle_type',
            default_value=['x500'],
            description='Vehicle type',
        ),
        DeclareLaunchArgument(
            'script_test',
            default_value=[''],
            description='Script to test',
        ),
        SetEnvironmentVariable('PX4_GZ_WORLD', ''),
        SetEnvironmentVariable('PX4_GZ_MODEL', ''),
        included_launch,
        micro_ros_agent,
        KeepAliveProc(),
        # Tell launch to start the test
        ReadyToTest(),
        autostart_env_var
    ])

    [run_px4_1, spawn_1] = create_px4_instance('1')
    [run_px4_2, spawn_2] = create_px4_instance('2')
    ld.add_action(spawn_1)
    ld.add_action(spawn_2)

    context = {'included_launch': included_launch,
               'micro_ros_agent': micro_ros_agent,
               'run_px4_1': run_px4_1,
               'run_px4_2': run_px4_2}

    return ld, context


class TestFixture(unittest.TestCase):

    def test_arm(self, launch_service, proc_info, proc_output):
        try:
            for i in range(1, 3):
                proc_output.assertWaitFor(f'INFO  [px4] instance: {i}',
                                          timeout=100, stream='stdout')
                proc_output.assertWaitFor('Ready for takeoff!',
                                          timeout=100, stream='stdout')

            proc_action = Node(
                package='vehicle_gateway_integration_test',
                executable=LaunchConfiguration('script_test'),
                output='screen'
            )

            with launch_testing.tools.launch_process(
                launch_service, proc_action, proc_info, proc_output
            ):
                proc_info.assertWaitForShutdown(process=proc_action, timeout=300)
                launch_testing.asserts.assertExitCodes(proc_info, process=proc_action,
                                                       allowable_exit_codes=[0])
        except AssertionError as e:
            print(e.what())
        finally:
            for i in range(1, 3):
                # shutdown px4
                p = subprocess.Popen(split(f'px4-shutdown --instance {i}'),
                                     stdout=subprocess.PIPE,
                                     stderr=subprocess.PIPE)
                p.wait()
            for proc in psutil.process_iter():
                # check whether the process name matches
                if proc.name() == 'ruby' or proc.name() == 'micro_ros_agent':
                    proc.kill()


# These tests are run after the processes in generate_test_description() have shutdown.
@launch_testing.post_shutdown_test()
class TestHelloWorldShutdown(unittest.TestCase):

    def test_exit_codes(self, proc_info, run_px4_1, run_px4_2):
        """Check if the processes exited normally."""
        launch_testing.asserts.assertExitCodes(proc_info, process=run_px4_1,
                                               allowable_exit_codes=[0])
        launch_testing.asserts.assertExitCodes(proc_info, process=run_px4_2,
                                               allowable_exit_codes=[0])
