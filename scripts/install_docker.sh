#!/usr/bin/env bash

remove_conflicting_docker_packages()
{
  log "Removing conflicting Docker packages"
  local packages
  packages="$(dpkg --get-selections docker.io docker-compose docker-compose-v2 docker-doc docker-buildx podman-docker containerd runc 2>/dev/null \
    | cut -f1 \
    | xargs || true)"

  if [ -n "${packages}" ]; then
    sudo_cmd apt remove -y ${packages}
  fi
}

install_docker_engine()
{
  remove_conflicting_docker_packages

  log "Installing Docker apt repository"
  sudo_cmd apt update -y
  apt_install ca-certificates curl
  sudo_cmd install -m 0755 -d /etc/apt/keyrings
  sudo_cmd curl -fsSL https://download.docker.com/linux/ubuntu/gpg -o /etc/apt/keyrings/docker.asc
  sudo_cmd chmod a+r /etc/apt/keyrings/docker.asc

  # shellcheck disable=SC1091
  . /etc/os-release
  local ubuntu_codename="${UBUNTU_CODENAME:-${VERSION_CODENAME}}"
  local architecture
  architecture="$(dpkg --print-architecture)"
  local docker_sources_file
  docker_sources_file="$(mktemp)"

  cat > "${docker_sources_file}" << EOF
Types: deb
URIs: https://download.docker.com/linux/ubuntu
Suites: ${ubuntu_codename}
Components: stable
Architectures: ${architecture}
Signed-By: /etc/apt/keyrings/docker.asc
EOF

  sudo_cmd cp "${docker_sources_file}" /etc/apt/sources.list.d/docker.sources
  rm -f "${docker_sources_file}"

  sudo_cmd apt update -y

  log "Installing Docker packages"
  apt_install docker-ce docker-ce-cli containerd.io docker-buildx-plugin docker-compose-plugin x11-xserver-utils
}

configure_docker_postinstall()
{
  log "Configuring Docker post-install"
  sudo_cmd systemctl start docker || true
  sudo_cmd systemctl enable docker.service
  sudo_cmd systemctl enable containerd.service

  if ! getent group docker >/dev/null; then
    sudo_cmd groupadd docker
  fi

  sudo_cmd usermod -aG docker "${USER}"

  log "Verifying Docker installation"
  sudo_cmd docker run hello-world
}

docker_cmd()
{
  if docker info >/dev/null 2>&1; then
    docker "$@"
    return
  fi

  sudo_cmd docker "$@"
}

prepare_docker_environment()
{
  log "Preparing Docker .env"
  "${DOCKER_ENV_SCRIPT}" "${ROS_DISTRO}"
}

build_docker_image()
{
  log "Building Docker image robogait_gui:${ROS_DISTRO}"
  cd "${ROOT_DIR}"
  docker_cmd build --build-arg "ROS_DISTRO=${ROS_DISTRO}" -t "robogait_gui:${ROS_DISTRO}" -f docker/DockerFile .
}
