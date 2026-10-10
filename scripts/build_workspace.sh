#!/usr/bin/env bash

build_workspace()
{
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

  local packages=(command_executor_msgs navigation_pkg command_executor robogait_gui)
  local nav2_compat_args=(--packages-ignore nav2_msgs)

  if [ "${ROS_DISTRO}" = "lyrical" ]; then
    packages=(nav2_msgs "${packages[@]}")
    nav2_compat_args=()
  fi

  colcon build --merge-install --base-paths src \
    "${nav2_compat_args[@]}" \
    --packages-select "${packages[@]}"
}

configure_development_shell()
{
  log "Configuring .bashrc"
  append_bashrc_line "source /opt/ros/${ROS_DISTRO}/setup.bash"
  append_bashrc_line "source ${ROOT_DIR}/install/setup.bash"
}
