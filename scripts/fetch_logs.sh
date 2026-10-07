#!/usr/bin/env bash
# 内部PCの logs/ を Mac の logs/ に同期 (削除はしない)
set -eu
source "$(dirname "$0")/common.sh"
rsync -az "${G1_SSH}:${G1_REMOTE_DIR}/logs/" "${ROOT_DIR}/logs/"
ls -lt "${ROOT_DIR}/logs" | head -6
