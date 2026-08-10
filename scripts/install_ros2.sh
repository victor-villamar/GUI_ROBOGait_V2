#!/usr/bin/env bash

add_ros2_repository()
{
  log "Adding ROS2 GPG key and repository"
  sudo_cmd apt update -y
  apt_install curl software-properties-common
  sudo_cmd add-apt-repository universe -y
  sudo_cmd curl -sSL https://raw.githubusercontent.com/ros/rosdistro/master/ros.key -o /usr/share/keyrings/ros-archive-keyring.gpg

  # shellcheck disable=SC1091
  . /etc/os-release
  local ubuntu_codename="${UBUNTU_CODENAME:-${VERSION_CODENAME}}"
  local architecture
  architecture="$(dpkg --print-architecture)"
  local ros_sources_file
  ros_sources_file="$(mktemp)"

  echo "deb [arch=${architecture} signed-by=/usr/share/keyrings/ros-archive-keyring.gpg] http://packages.ros.org/ros2/ubuntu ${ubuntu_codename} main" \
    > "${ros_sources_file}"
  sudo_cmd cp "${ros_sources_file}" /etc/apt/sources.list.d/ros2.list
  rm -f "${ros_sources_file}"

  sudo_cmd apt update -y
}

install_ros2_dependencies()
{
  log "Installing ROS2 dependencies"
  apt_install \
    "ros-${ROS_DISTRO}-ros-base" \
    "ros-${ROS_DISTRO}-ament-cmake" \
    "ros-${ROS_DISTRO}-ament-index-cpp" \
    "ros-${ROS_DISTRO}-rclcpp" \
    "ros-${ROS_DISTRO}-rclcpp-action" \
    "ros-${ROS_DISTRO}-geometry-msgs" \
    "ros-${ROS_DISTRO}-sensor-msgs" \
    "ros-${ROS_DISTRO}-nav-msgs" \
    "ros-${ROS_DISTRO}-map-msgs" \
    "ros-${ROS_DISTRO}-nav2-msgs" \
    "ros-${ROS_DISTRO}-std-srvs" \
    "ros-${ROS_DISTRO}-tf2" \
    "ros-${ROS_DISTRO}-tf2-ros" \
    "ros-${ROS_DISTRO}-tf2-geometry-msgs" \
    "ros-${ROS_DISTRO}-rosidl-default-generators" \
    "ros-${ROS_DISTRO}-rosidl-default-runtime"
}
