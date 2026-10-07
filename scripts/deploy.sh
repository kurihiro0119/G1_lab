#!/usr/bin/env bash
# ワークスペース一式を G1 内部PCへ転送 (記事の scp 手順に相当, 差分転送)
set -eu
source "$(dirname "$0")/common.sh"
ssh "${G1_SSH}" "mkdir -p '${G1_REMOTE_DIR}/logs'"
rsync -az --delete \
  --exclude 'build/' --exclude 'build-*/' --exclude 'logs/' --exclude 'tools/.venv/' --exclude '.git/' --exclude '.DS_Store' \
  "${ROOT_DIR}/" "${G1_SSH}:${G1_REMOTE_DIR}/"
echo "deployed -> ${G1_SSH}:${G1_REMOTE_DIR}"
