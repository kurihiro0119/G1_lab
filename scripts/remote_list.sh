#!/usr/bin/env bash
# G1 内部PC上のビルド済み G1 用プログラム一覧
set -eu
source "$(dirname "$0")/common.sh"
ssh "${G1_SSH}" "cd '${G1_REMOTE_DIR}/build' && echo '[自作] bin/' && ls bin && echo && echo '[公式サンプル] third_party/unitree_sdk2/bin/ (g1*)' && ls third_party/unitree_sdk2/bin | grep '^g1'"
