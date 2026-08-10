#!/usr/bin/env bash

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SCRIPTS_DIR="${ROOT_DIR}/scripts"

# shellcheck source=scripts/common.sh
source "${SCRIPTS_DIR}/common.sh"
# shellcheck source=scripts/install_ros2.sh
source "${SCRIPTS_DIR}/install_ros2.sh"
# shellcheck source=scripts/install_qt6.sh
source "${SCRIPTS_DIR}/install_qt6.sh"
# shellcheck source=scripts/install_dev_dependencies.sh
source "${SCRIPTS_DIR}/install_dev_dependencies.sh"
# shellcheck source=scripts/build_workspace.sh
source "${SCRIPTS_DIR}/build_workspace.sh"
# shellcheck source=scripts/install_docker.sh
source "${SCRIPTS_DIR}/install_docker.sh"
# shellcheck source=scripts/install_docker_service.sh
source "${SCRIPTS_DIR}/install_docker_service.sh"

install_development_computer()
{
  require_ubuntu
  choose_ros_distro
  validate_ros_distro_for_host
  request_password
  configure_locales
  add_ros2_repository
  install_development_dependencies
  build_workspace
  configure_development_shell
  log "Development computer installation completed"
}

install_application_device()
{
  require_ubuntu
  choose_ros_distro
  request_password
  install_docker_engine
  configure_docker_postinstall
  prepare_docker_environment
  build_docker_image
  install_docker_user_service
  log "Application device installation completed"
  echo "[install] Docker instalado. Para usar docker sin sudo y para que funcione el servicio, cierra sesion y vuelve a entrar o reinicia."
}

install_docker_only()
{
  require_ubuntu
  choose_ros_distro
  request_password
  install_docker_engine
  configure_docker_postinstall
  prepare_docker_environment
  build_docker_image
  log "Docker installation completed"
  echo "[install] Docker instalado. Para usar docker sin sudo y para que funcione el servicio, cierra sesion y vuelve a entrar o reinicia."
}

main()
{
  echo "ROBOGait installer"
  echo "  1. Instalacion de ordenador de desarrollo"
  echo "  2. Instalacion de aplicacion en un dispositivo"
  echo "  3. Instalacion de docker"
  read -r -p "Opcion [1/2/3]: " option

  case "${option}" in
    1)
      install_development_computer
      ;;
    2)
      install_application_device
      ;;
    3)
      install_docker_only
      ;;
    *)
      die "Opcion no valida"
      ;;
  esac
}

main "$@"
