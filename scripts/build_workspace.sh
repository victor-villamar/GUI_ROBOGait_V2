#!/usr/bin/env bash

build_selected_workspace()
{
  local include_command_executor="$1"

  log "Building ROBOGait workspace"

  # ROS setup scripts read optional environment variables that may be unset.
  # Temporarily disable nounset because install.sh runs with `set -u`.
  local nounset_was_enabled=false
  if [[ $- == *u* ]]; then
    nounset_was_enabled=true
    set +u
  fi

  # shellcheck disable=SC1090
  source "/opt/ros/${ROS_DISTRO}/setup.bash"

  if [ "${nounset_was_enabled}" = true ]; then
    set -u
  fi

  cd "${ROOT_DIR}"

  local packages=(command_executor_msgs navigation_pkg robogait_gui)
  local nav2_compat_args=(--packages-ignore nav2_msgs)

  if [ "${include_command_executor}" = true ]; then
    packages=(command_executor_msgs navigation_pkg command_executor robogait_gui)
  fi

  if [ "${ROS_DISTRO}" = "lyrical" ]; then
    packages=(nav2_msgs "${packages[@]}")
    nav2_compat_args=()
  fi

  colcon build --merge-install --base-paths src \
    "${nav2_compat_args[@]}" \
    --packages-select "${packages[@]}"
}

build_workspace()
{
  build_selected_workspace true
}

build_application_workspace()
{
  build_selected_workspace false
}

configure_development_shell()
{
  log "Configuring .bashrc"
  append_bashrc_line "source /opt/ros/${ROS_DISTRO}/setup.bash"
  append_bashrc_line "source ${ROOT_DIR}/install/setup.bash"
}