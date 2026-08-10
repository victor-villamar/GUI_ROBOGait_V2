#!/usr/bin/env bash

install_docker_user_service()
{
  log "Installing user systemd service"
  local user_systemd_dir="${HOME}/.config/systemd/user"
  local service_file="${user_systemd_dir}/robogait-gui-docker.service"

  mkdir -p "${user_systemd_dir}"

  cat > "${service_file}" << EOF
[Unit]
Description=ROBOGait GUI Docker Compose
After=graphical-session.target
Wants=graphical-session.target

[Service]
Type=simple
WorkingDirectory=${DOCKER_DIR}
Environment=ROS_DISTRO=${ROS_DISTRO}
Environment=DISPLAY=${DISPLAY:-:0}
ExecStartPre=/usr/bin/xhost +local:docker
ExecStart=/usr/bin/docker compose up robogait_gui
ExecStop=/usr/bin/docker compose down
Restart=on-failure
RestartSec=5

[Install]
WantedBy=default.target
EOF

  systemctl --user daemon-reload
  systemctl --user enable robogait-gui-docker.service

  echo "[install] Servicio instalado: ${service_file}"
  echo "[install] Para arrancarlo ahora: systemctl --user start robogait-gui-docker.service"
}
