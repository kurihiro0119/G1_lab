#!/usr/bin/env bash
# G1 内部PC上でビルド済みプログラムを実行
#   usage: scripts/remote_run.sh [--sudo] <program> [extra args...]
#   例:    scripts/remote_run.sh g1_state_monitor
#          scripts/remote_run.sh --sudo g1_dual_arm_example
# "free(): invalid pointer" が出る場合は --sudo を付ける (記事の注意事項)
set -eu
source "$(dirname "$0")/common.sh"
SUDO=""
if [ "${1:-}" = "--sudo" ]; then SUDO="sudo"; shift; fi
PROG="${1:?program name required (scripts/remote_list.sh で確認)}"; shift
# 空白を含む引数 (tts "hello world" など) をそのままリモートへ渡す
ARGS=""; [ $# -gt 0 ] && ARGS="$(printf ' %q' "$@")"
# 自作: build/bin, 公式サンプル: build/third_party/unitree_sdk2/bin
ssh -t "${G1_SSH}" "cd '${G1_REMOTE_DIR}/build' && \
  for d in bin third_party/unitree_sdk2/bin; do \
    if [ -x \"\$d/${PROG}\" ]; then exec ${SUDO} \"./\$d/${PROG}\" ${G1_IFACE}${ARGS}; fi; \
  done; echo 'not found: ${PROG}' >&2; exit 1"
