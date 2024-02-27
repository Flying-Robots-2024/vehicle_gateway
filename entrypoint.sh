#!/bin/bash

set -e
source /opt/ros/humble/setup.bash
source /home/vehicle_gateway/install/local_setup.bash
exec "$@"
