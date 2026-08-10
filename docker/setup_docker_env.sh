#!/usr/bin/env bash

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ENV_FILE="${SCRIPT_DIR}/.env"
ROS_DISTRO="${1:-${ROS_DISTRO:-humble}}"

get_group_gid()
{
  local group_name="$1"
  local default_gid="$2"
  local group_entry

  group_entry="$(getent group "${group_name}" || true)"
  if [ -n "${group_entry}" ]; then
    echo "${group_entry}" | cut -d: -f3
    return
  fi

  echo "${default_gid}"
}

VIDEO_GID="$(get_group_gid video 44)"
RENDER_GID="$(get_group_gid render 110)"

{
  echo "ROS_DISTRO=${ROS_DISTRO}"
  echo "VIDEO_GID=${VIDEO_GID}"
  echo "RENDER_GID=${RENDER_GID}"
} > "${ENV_FILE}"

echo "[setup_docker_env] Docker environment written to ${ENV_FILE}"
echo "[setup_docker_env] ROS_DISTRO=${ROS_DISTRO} VIDEO_GID=${VIDEO_GID} RENDER_GID=${RENDER_GID}"
