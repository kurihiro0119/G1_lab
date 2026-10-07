#!/usr/bin/env bash
# 実験前チェック: 疎通 → ビルド物の有無 → 健全性 (g1_health_check)
set -u
source "$(dirname "$0")/common.sh"

echo "== 1. 疎通 =="
if ! ssh -o BatchMode=yes -o ConnectTimeout=5 "${G1_SSH}" true 2>/dev/null; then
  echo "NG: ${G1_SSH} に SSH できません (scripts/check_network.sh で確認)"; exit 1
fi
echo "OK"

echo "== 2. ビルド物 =="
if ! ssh "${G1_SSH}" "test -x '${G1_REMOTE_DIR}/build/bin/g1_health_check'"; then
  echo "NG: 未ビルドです (scripts/remote_build.sh)"; exit 1
fi
echo "OK"

echo "== 3. 健全性 =="
"$(dirname "$0")/remote_run.sh" g1_health_check 3
