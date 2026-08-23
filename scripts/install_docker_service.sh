#!/usr/bin/env bash

install_docker_user_service()
{
  log "Installing user systemd service"
  local user_systemd_dir="${HOME}/.config/systemd/user"
  local service_file="${user_systemd_dir}/robogait-gui-docker.service"
  local user_bin_dir="${HOME}/.local/bin"
  local launcher_file="${user_bin_dir}/start-robogait-gui-docker"
  local autostart_dir="${HOME}/.config/autostart"
  local autostart_file="${autostart_dir}/robogait-gui-docker.desktop"
  local applications_dir="${HOME}/.local/share/applications"
  local application_file="${applications_dir}/robogait-gui.desktop"
  local icons_dir="${HOME}/.local/share/icons"
  local icon_file="${icons_dir}/robogait-gui.png"
  local source_icon="${ROOT_DIR}/src/robogait_gui/resources/Logos/robogait_logo.png"

  mkdir -p "${user_systemd_dir}" "${user_bin_dir}" "${autostart_dir}" "${applications_dir}" "${icons_dir}"
  systemctl --user disable --now robogait-gui-docker.service 2>/dev/null || true

  if [ ! -f "${source_icon}" ]; then
    die "No se ha encontrado el icono de ROBOGait: ${source_icon}"
  fi

  install -m 0644 "${source_icon}" "${icon_file}"

  cat > "${service_file}" << EOF
[Unit]
Description=ROBOGait GUI Docker Compose
StartLimitIntervalSec=60
StartLimitBurst=3

[Service]
Type=simple
WorkingDirectory=${DOCKER_DIR}
Environment=ROS_DISTRO=${ROS_DISTRO}
ExecStartPre=/usr/bin/xhost +local:docker
ExecStart=/usr/bin/docker compose up robogait_gui
ExecStop=/usr/bin/docker compose down
Restart=on-failure
RestartSec=5
EOF

  cat > "${launcher_file}" << 'EOF'
#!/usr/bin/env bash

set -euo pipefail

graphical_environment=(DISPLAY XDG_RUNTIME_DIR)

if [ -n "${XAUTHORITY:-}" ]; then
  graphical_environment+=(XAUTHORITY)
fi

if [ -n "${WAYLAND_DISPLAY:-}" ]; then
  graphical_environment+=(WAYLAND_DISPLAY)
fi

systemctl --user import-environment "${graphical_environment[@]}"
systemctl --user reset-failed robogait-gui-docker.service 2>/dev/null || true
systemctl --user start robogait-gui-docker.service

if [[ ":${XDG_CURRENT_DESKTOP:-}:" == *":GNOME:"* ]] && command -v gsettings >/dev/null 2>&1; then
  favorites="$(gsettings get org.gnome.shell favorite-apps 2>/dev/null || true)"
  desktop_id="robogait-gui.desktop"

  if [ -n "${favorites}" ] && [[ "${favorites}" != *"'${desktop_id}'"* ]]; then
    if [ "${favorites}" = "[]" ]; then
      favorites="['${desktop_id}']"
    else
      favorites="${favorites%]}, '${desktop_id}']"
    fi

    gsettings set org.gnome.shell favorite-apps "${favorites}" >/dev/null 2>&1 || true
  fi
fi
EOF

  chmod +x "${launcher_file}"

  cat > "${autostart_file}" << EOF
[Desktop Entry]
Type=Application
Name=ROBOGait GUI
Comment=Start ROBOGait GUI with Docker Compose
Exec=${launcher_file}
Terminal=false
X-GNOME-Autostart-enabled=true
EOF

  cat > "${application_file}" << EOF
[Desktop Entry]
Type=Application
Name=ROBOGait GUI
Comment=Start ROBOGait GUI with Docker Compose
Exec=${launcher_file}
Icon=${icon_file}
Terminal=false
Categories=Utility;
StartupNotify=true
StartupWMClass=robogait_gui
EOF

  systemctl --user daemon-reload

  echo "[install] Servicio instalado: ${service_file}"
  echo "[install] Inicio gráfico instalado: ${autostart_file}"
  echo "[install] Lanzador de aplicación instalado: ${application_file}"
  echo "[install] El servicio se iniciará al abrir la próxima sesión gráfica."
  echo "[install] Para arrancarlo ahora desde esta sesión: ${launcher_file}"
}

install_docker_service_main()
{
  ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

  # shellcheck source=scripts/common.sh
  source "${ROOT_DIR}/scripts/common.sh"

  if [ -z "${ROS_DISTRO}" ] && [ -f "${DOCKER_DIR}/.env" ]; then
    # shellcheck disable=SC1091
    source "${DOCKER_DIR}/.env"
  fi

  ROS_DISTRO="${1:-${ROS_DISTRO:-}}"
  case "${ROS_DISTRO}" in
    humble|jazzy)
      ;;
    *)
      die "Uso: $0 <humble|jazzy>. También puede generarse antes docker/.env."
      ;;
  esac

  install_docker_user_service
}

if [ "${BASH_SOURCE[0]}" = "$0" ]; then
  set -euo pipefail
  install_docker_service_main "$@"
fi
