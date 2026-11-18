#!/bin/bash

set -e

# Source the ROS 2 robogait installation
source "/opt/ros/robogait/setup.bash"

# Execute the command passed into the container
exec "$@"