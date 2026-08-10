#!/usr/bin/env bash

PASSWORD="${PASSWORD:-}"
ROS_DISTRO="${ROS_DISTRO:-}"
DOCKER_DIR="${ROOT_DIR}/docker"
DOCKER_ENV_SCRIPT="${DOCKER_DIR}/setup_docker_env.sh"

log()
{
  echo -e "\n-------- $* --------\n"
}

die()
{
  echo "[install] ERROR: $*" >&2
  exit 1
}

sudo_cmd()
{
  echo "${PASSWORD}" | sudo -S "$@"
}

require_ubuntu()
{
  if [ ! -f /etc/os-release ]; then
    die "No se ha encontrado /etc/os-release"
  fi

  # shellcheck disable=SC1091
  . /etc/os-release
  if [ "${ID:-}" != "ubuntu" ]; then
    die "Este instalador esta preparado para Ubuntu. Sistema detectado: ${ID:-unknown}"
  fi
}

request_password()
{
  if [ -z "${PASSWORD+x}" ] || [ -z "${PASSWORD}" ]; then
    read -r -s -p "Enter PASSWORD: " PASSWORD
    echo
  fi

  sudo_cmd -v >/dev/null
}

choose_ros_distro()
{
  echo "Selecciona la version de ROS2:"
  echo "  1. humble"
  echo "  2. jazzy"
  read -r -p "Opcion [1/2]: " option

  case "${option}" in
    1)
      ROS_DISTRO="humble"
      ;;
    2)
      ROS_DISTRO="jazzy"
      ;;
    *)
      die "Opcion no valida"
      ;;
  esac
}

validate_ros_distro_for_host()
{
  # shellcheck disable=SC1091
  . /etc/os-release
  local ubuntu_codename="${UBUNTU_CODENAME:-${VERSION_CODENAME}}"

  if [ "${ROS_DISTRO}" = "humble" ] && [ "${ubuntu_codename}" != "jammy" ]; then
    die "ROS2 Humble por apt esta soportado para Ubuntu 22.04 jammy. Sistema detectado: ${ubuntu_codename}"
  fi

  if [ "${ROS_DISTRO}" = "jazzy" ] && [ "${ubuntu_codename}" != "noble" ]; then
    die "ROS2 Jazzy por apt esta soportado para Ubuntu 24.04 noble. Sistema detectado: ${ubuntu_codename}"
  fi
}

apt_install()
{
  sudo_cmd apt install -y "$@"
}

configure_locales()
{
  log "Verification and installing locales"
  sudo_cmd apt update -y
  apt_install locales
  sudo_cmd locale-gen en_US en_US.UTF-8
  sudo_cmd update-locale LC_ALL=en_US.UTF-8 LANG=en_US.UTF-8
  export LANG=en_US.UTF-8
}

append_bashrc_line()
{
  local line="$1"
  local bashrc="${HOME}/.bashrc"

  touch "${bashrc}"
  if ! grep -Fqx "${line}" "${bashrc}"; then
    echo "${line}" >> "${bashrc}"
  fi
}
