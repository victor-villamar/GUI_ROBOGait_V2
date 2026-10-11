#!/usr/bin/env bash

add_native_launcher_to_gnome_favorites()
{
  if ! command -v gsettings >/dev/null 2>&1; then
    return
  fi

  local desktop_id="robogait-gui-native.desktop"
  local favorites
  favorites="$(gsettings get org.gnome.shell favorite-apps 2>/dev/null || true)"

  if [ -z "${favorites}" ] || [[ "${favorites}" == *"'${desktop_id}'"* ]]; then
    return
  fi

  case "${favorites}" in
    "[]"|"@as []")
      favorites="['${desktop_id}']"
      ;;
    *)
      favorites="${favorites%]}, '${desktop_id}']"
      ;;
  esac

  gsettings set org.gnome.shell favorite-apps "${favorites}" >/dev/null 2>&1 || true
}

install_native_runtime_launcher()
{
  local user_bin_dir="${HOME}/.local/bin"
  local runtime_file="${user_bin_dir}/run-robogait-gui-native"

  mkdir -p "${user_bin_dir}"

  cat > "${runtime_file}" << EOF
#!/usr/bin/env bash

set -eo pipefail

# shellcheck disable=SC1091
source "/opt/ros/${ROS_DISTRO}/setup.bash"
# shellcheck disable=SC1091
source "${ROOT_DIR}/install/setup.bash"

set -u
exec ros2 run robogait_gui robogait_gui
EOF

  chmod +x "${runtime_file}"
  printf '%s\n' "${runtime_file}"
}

install_native_desktop_launcher()
{
  local launch_file="$1"
  local applications_dir="${HOME}/.local/share/applications"
  local application_file="${applications_dir}/robogait-gui-native.desktop"
  local icons_dir="${HOME}/.local/share/icons"
  local icon_file="${icons_dir}/robogait-gui-native.png"
  local source_icon="${ROOT_DIR}/src/robogait_gui/resources/Logos/robogait.png"

  mkdir -p "${applications_dir}" "${icons_dir}"

  if [ ! -f "${source_icon}" ]; then
    die "No se ha encontrado el icono nativo de ROBOGait: ${source_icon}"
  fi

  install -m 0644 "${source_icon}" "${icon_file}"

  cat > "${application_file}" << EOF
[Desktop Entry]
Type=Application
Name=ROBOGait GUI (Native)
Comment=Start the native ROBOGait GUI application
Exec=${launch_file}
TryExec=${launch_file}
Icon=${icon_file}
Terminal=false
Categories=Utility;
StartupNotify=true
StartupWMClass=robogait_gui
EOF

  add_native_launcher_to_gnome_favorites

  echo "[install] Lanzador nativo instalado: ${application_file}"
  echo "[install] Icono nativo instalado: ${icon_file}"
}

install_native_application_launcher()
{
  log "Installing native application launcher"
  local runtime_file
  runtime_file="$(install_native_runtime_launcher)"
  install_native_desktop_launcher "${runtime_file}"
}

install_native_user_service()
{
  log "Installing native user systemd service"
  local user_systemd_dir="${HOME}/.config/systemd/user"
  local service_file="${user_systemd_dir}/robogait-gui-native.service"
  local user_bin_dir="${HOME}/.local/bin"
  local service_launcher_file="${user_bin_dir}/start-robogait-gui-native"
  local autostart_dir="${HOME}/.config/autostart"
  local autostart_file="${autostart_dir}/robogait-gui-native.desktop"
  local runtime_file
  runtime_file="$(install_native_runtime_launcher)"

  mkdir -p "${user_systemd_dir}" "${user_bin_dir}" "${autostart_dir}"
  systemctl --user disable --now robogait-gui-native.service 2>/dev/null || true

  cat > "${service_file}" << EOF
[Unit]
Description=ROBOGait GUI Native
StartLimitIntervalSec=60
StartLimitBurst=3

[Service]
Type=simple
ExecStart=${runtime_file}
Restart=on-failure
RestartSec=5
EOF

  cat > "${service_launcher_file}" << 'EOF'
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
systemctl --user reset-failed robogait-gui-native.service 2>/dev/null || true
systemctl --user start robogait-gui-native.service
EOF

  chmod +x "${service_launcher_file}"

  cat > "${autostart_file}" << EOF
[Desktop Entry]
Type=Application
Name=ROBOGait GUI (Native)
Comment=Start the native ROBOGait GUI systemd service
Exec=${service_launcher_file}
Terminal=false
X-GNOME-Autostart-enabled=true
EOF

  install_native_desktop_launcher "${service_launcher_file}"
  systemctl --user daemon-reload

  echo "[install] Servicio nativo instalado: ${service_file}"
  echo "[install] Inicio gráfico nativo instalado: ${autostart_file}"
  echo "[install] El servicio se iniciará al abrir la próxima sesión gráfica."
  echo "[install] Para arrancarlo ahora desde esta sesión: ${service_launcher_file}"
}