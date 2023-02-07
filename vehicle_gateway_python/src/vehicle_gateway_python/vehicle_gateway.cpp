// Copyright 2023 Open Source Robotics Foundation, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <pybind11/pybind11.h>

#include "exceptions.hpp"
#include "vehicle_gateway.hpp"

namespace vehicle_gateway_python
{
VehicleGatewayPython::VehicleGatewayPython(
  const std::vector<std::string> & _args,
  const std::string & _plugin_name)
{
  this->loader_ = std::make_shared<pluginlib::ClassLoader<vehicle_gateway::VehicleGateway>>(
    "vehicle_gateway", "vehicle_gateway::VehicleGateway");

  if (_plugin_name == "px4") {
    this->gateway_ = this->loader_->createSharedInstance(
      "vehicle_gateway_px4::VehicleGatewayPX4");
    if (!this->gateway_) {
      throw InvalidHandle("cannot use create the requested API");
    }
  } else {
    throw InvalidHandle("cannot use create the requested API");
  }

  const char ** argv = new const char *[_args.size()];

  for (unsigned index = 0; index < _args.size(); index++) {
    argv[index] = _args[index].c_str();
  }

  this->gateway_->init(_args.size(), argv);
  delete[] argv;
}

void VehicleGatewayPython::Destroy()
{
  if (this->gateway_) {
    this->gateway_->destroy();
    this->gateway_ = nullptr;
  }
  if (this->loader_) {
    this->loader_->unloadLibraryForClass(
      "vehicle_gateway_px4::VehicleGatewayPX4");
    this->loader_ = nullptr;
  }
}

void VehicleGatewayPython::Arm()
{
  this->gateway_->arm();
}

void VehicleGatewayPython::Disarm()
{
  this->gateway_->disarm();
}

void VehicleGatewayPython::TransitionToMultiCopter()
{
  this->gateway_->transition_to_mc();
}

void VehicleGatewayPython::TransitionToFixedWings()
{
  this->gateway_->transition_to_fw();
}

void VehicleGatewayPython::PublishLocalPositionSetpoint(float x, float y, float z)
{
  this->gateway_->set_local_position_setpoint(x, y, z);
}

void VehicleGatewayPython::SetOffboardControlMode(bool is_trajectory)
{
  this->gateway_->set_offboard_control_mode(is_trajectory);
}

void VehicleGatewayPython::SetOffboardMode()
{
  this->gateway_->set_offboard_mode();
}

float VehicleGatewayPython::GetGroundSpeed()
{
  return this->gateway_->get_ground_speed();
}

vehicle_gateway::FLIGHT_MODE VehicleGatewayPython::GetFlightMode()
{
  return this->gateway_->get_flight_mode();
}

vehicle_gateway::VEHICLE_TYPE VehicleGatewayPython::GetVehicleType()
{
  return this->gateway_->get_vehicle_type();
}

vehicle_gateway::ARMING_STATE VehicleGatewayPython::GetArmingState()
{
  return this->gateway_->get_arming_state();
}

vehicle_gateway::ARM_DISARM_REASON VehicleGatewayPython::GetArmReason()
{
  return this->gateway_->get_arm_reason();
}

vehicle_gateway::ARM_DISARM_REASON VehicleGatewayPython::GetDisarmReason()
{
  return this->gateway_->get_disarm_reason();
}

vehicle_gateway::FAILURE VehicleGatewayPython::GetFailure()
{
  return this->gateway_->get_failure();
}

void VehicleGatewayPython::Takeoff()
{
  this->gateway_->takeoff();
}

void VehicleGatewayPython::Land()
{
  this->gateway_->land();
}

VehicleGatewayPython::~VehicleGatewayPython()
{
  this->Destroy();
}

void
define_vehicle_gateway(py::object module)
{
  py::class_<VehicleGatewayPython, Destroyable, std::shared_ptr<VehicleGatewayPython>>(
    module,
    "VehicleGatewayPython")
  .def(
    py::init<const std::vector<std::string> &,
    const std::string &>())
  .def(
    "destroy", &VehicleGatewayPython::Destroy,
    "Destroy object")
  .def(
    "arm", &VehicleGatewayPython::Arm,
    "Arm vehicle")
  .def(
    "disarm", &VehicleGatewayPython::Disarm,
    "Disarm vehicle")
  .def(
    "get_ground_speed", &VehicleGatewayPython::GetGroundSpeed,
    "Get ground speed")
  .def(
    "transition_to_fw", &VehicleGatewayPython::TransitionToFixedWings,
    "Transition to fixed wings")
  .def(
    "transition_to_mc", &VehicleGatewayPython::TransitionToMultiCopter,
    "Transition to multicopter")
  .def(
    "takeoff", &VehicleGatewayPython::Takeoff,
    "TakeOff")
  .def(
    "set_local_position_setpoint", &VehicleGatewayPython::PublishLocalPositionSetpoint,
    "PublishLocalPositionSetpoint")
  .def(
    "set_offboard_mode", &VehicleGatewayPython::SetOffboardMode,
    "SetOffboardMode")
  .def(
    "set_offboard_control_mode", &VehicleGatewayPython::SetOffboardControlMode,
    "SetOffboardControlMode")
  .def(
    "get_arm_reason", &VehicleGatewayPython::GetArmReason,
    "Get arm reason")
  .def(
    "get_disarm_reason", &VehicleGatewayPython::GetDisarmReason,
    "Get disarm reason")
  .def(
    "set_offboard_control_mode", &VehicleGatewayPython::SetOffboardControlMode,
    "SetOffboardControlMode")
  .def(
    "get_arming_state", &VehicleGatewayPython::GetArmingState,
    "Get arming state")
  .def(
    "get_flight_mode", &VehicleGatewayPython::GetFlightMode,
    "Get Flight mode")
  .def(
    "get_vehicle_type", &VehicleGatewayPython::GetVehicleType,
    "Get vehicle type")
  .def(
    "get_failure", &VehicleGatewayPython::GetFailure,
    "Get failure")
  .def(
    "land", &VehicleGatewayPython::Land,
    "Land");

  pybind11::enum_<vehicle_gateway::ARMING_STATE>(module, "ArmingState")
  .value("INIT", vehicle_gateway::ARMING_STATE::INIT)
  .value("STANDBY", vehicle_gateway::ARMING_STATE::STANDBY)
  .value("ARMED", vehicle_gateway::ARMING_STATE::ARMED)
  .value("STANDBY_ERROR", vehicle_gateway::ARMING_STATE::STANDBY_ERROR)
  .value("SHUTTEDDOWN", vehicle_gateway::ARMING_STATE::SHUTTEDDOWN)
  .value("IN_AIR_RESTORE", vehicle_gateway::ARMING_STATE::IN_AIR_RESTORE)
  .value("MAX", vehicle_gateway::ARMING_STATE::MAX)
  .export_values();

  pybind11::enum_<vehicle_gateway::ARM_DISARM_REASON>(module, "ArmDisarmReason")
  .value("TRANSITION_TO_STANDBY", vehicle_gateway::ARM_DISARM_REASON::TRANSITION_TO_STANDBY)
  .value("RC_STICK", vehicle_gateway::ARM_DISARM_REASON::RC_STICK)
  .value("RC_SWITCH", vehicle_gateway::ARM_DISARM_REASON::RC_SWITCH)
  .value("COMMAND_INTERNAL", vehicle_gateway::ARM_DISARM_REASON::COMMAND_INTERNAL)
  .value("COMMAND_EXTERNAL", vehicle_gateway::ARM_DISARM_REASON::COMMAND_EXTERNAL)
  .value("MISSION_START", vehicle_gateway::ARM_DISARM_REASON::MISSION_START)
  .value("SAFETY_BUTTON", vehicle_gateway::ARM_DISARM_REASON::SAFETY_BUTTON)
  .value("AUTO_DISARM_LAND", vehicle_gateway::ARM_DISARM_REASON::AUTO_DISARM_LAND)
  .value("AUTO_DISARM_PREFLIGHT", vehicle_gateway::ARM_DISARM_REASON::AUTO_DISARM_PREFLIGHT)
  .value("KILL_SWITCH", vehicle_gateway::ARM_DISARM_REASON::KILL_SWITCH)
  .value("LOCKDOWN", vehicle_gateway::ARM_DISARM_REASON::LOCKDOWN)
  .value("FAILURE_DETECTOR", vehicle_gateway::ARM_DISARM_REASON::FAILURE_DETECTOR)
  .value("SHUTDOWN", vehicle_gateway::ARM_DISARM_REASON::SHUTDOWN)
  .export_values();

  pybind11::enum_<vehicle_gateway::FAILURE>(module, "Failure")
  .value("NONE", vehicle_gateway::FAILURE::NONE)
  .value("ROLL", vehicle_gateway::FAILURE::ROLL)
  .value("PITCH", vehicle_gateway::FAILURE::PITCH)
  .value("ALT", vehicle_gateway::FAILURE::ALT)
  .value("EXT", vehicle_gateway::FAILURE::EXT)
  .value("ARM_ESC", vehicle_gateway::FAILURE::ARM_ESC)
  .value("BATTERY", vehicle_gateway::FAILURE::BATTERY)
  .value("IMBALANCED_PROP", vehicle_gateway::FAILURE::IMBALANCED_PROP)
  .value("MOTOR", vehicle_gateway::FAILURE::MOTOR)
  .export_values();

  pybind11::enum_<vehicle_gateway::VEHICLE_TYPE>(module, "VehicleType")
  .value("UNKNOWN", vehicle_gateway::VEHICLE_TYPE::UNKNOWN)
  .value("ROTARY_WING", vehicle_gateway::VEHICLE_TYPE::ROTARY_WING)
  .value("FIXED_WING", vehicle_gateway::VEHICLE_TYPE::FIXED_WING)
  .value("ROVER", vehicle_gateway::VEHICLE_TYPE::ROVER)
  .value("AIRSHIP", vehicle_gateway::VEHICLE_TYPE::AIRSHIP)
  .export_values();

  pybind11::enum_<vehicle_gateway::FLIGHT_MODE>(module, "FlightMode")
  .value("MANUAL", vehicle_gateway::FLIGHT_MODE::MANUAL)
  .value("ALTCTL", vehicle_gateway::FLIGHT_MODE::ALTCTL)
  .value("POSCTL", vehicle_gateway::FLIGHT_MODE::POSCTL)
  .value("AUTO_MISSION", vehicle_gateway::FLIGHT_MODE::AUTO_MISSION)
  .value("AUTO_LOITER", vehicle_gateway::FLIGHT_MODE::AUTO_LOITER)
  .value("AUTO_RTL", vehicle_gateway::FLIGHT_MODE::AUTO_RTL)
  .value("ACRO", vehicle_gateway::FLIGHT_MODE::ACRO)
  .value("DESCEND", vehicle_gateway::FLIGHT_MODE::DESCEND)
  .value("TERMINATION", vehicle_gateway::FLIGHT_MODE::TERMINATION)
  .value("OFFBOARD", vehicle_gateway::FLIGHT_MODE::OFFBOARD)
  .value("STAB", vehicle_gateway::FLIGHT_MODE::STAB)
  .value("AUTO_TAKEOFF", vehicle_gateway::FLIGHT_MODE::AUTO_TAKEOFF)
  .value("AUTO_LAND", vehicle_gateway::FLIGHT_MODE::AUTO_LAND)
  .value("AUTO_FOLLOW_TARGET", vehicle_gateway::FLIGHT_MODE::AUTO_FOLLOW_TARGET)
  .value("AUTO_PRECLAND", vehicle_gateway::FLIGHT_MODE::AUTO_PRECLAND)
  .value("ORBIT", vehicle_gateway::FLIGHT_MODE::ORBIT)
  .value("AUTO_VTOL_TAKEOFF", vehicle_gateway::FLIGHT_MODE::AUTO_VTOL_TAKEOFF)
  .value("UNKNOWN_MODE", vehicle_gateway::FLIGHT_MODE::UNKNOWN_MODE)
  .export_values();
}
}  // namespace vehicle_gateway_python