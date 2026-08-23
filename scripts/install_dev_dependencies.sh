#!/usr/bin/env bash

install_system_build_tools()
{
  log "Installing system build tools"
  apt_install \
    build-essential \
    cmake \
    g++ \
    curl \
    wget \
    python3-pip \
    python3-dev \
    python3-colcon-common-extensions \
    python3-rosdep \
    unzip \
    nano \
    net-tools \
    iputils-ping \
    libyaml-cpp-dev \
    libboost-dev \
    libboost-system-dev \
    libboost-filesystem-dev
}

install_database_development_tools()
{
  log "Installing optional database inspection tools"
  apt_install \
    sqlite3 \
    sqlitebrowser
}

install_development_dependencies()
{
  install_system_build_tools
  install_database_development_tools
  install_qt6_dependencies
  install_ros2_dependencies
}
