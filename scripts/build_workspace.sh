#!/usr/bin/env bash

build_workspace()
{
  log "Building ROBOGait workspace"
  # shellcheck disable=SC1090
  source "/opt/ros/${ROS_DISTRO}/setup.bash"
  cd "${ROOT_DIR}"
  colcon build --merge-install --base-paths src --packages-select command_executor_msgs navigation_pkg command_executor robogait_gui
}

configure_development_shell()
{
  log "Configuring .bashrc"
  append_bashrc_line "source /opt/ros/${ROS_DISTRO}/setup.bash"
  append_bashrc_line "source ${ROOT_DIR}/install/setup.bash"
}
