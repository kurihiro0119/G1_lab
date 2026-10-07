#!/usr/bin/env bash
# G1 内部PC上で cmake / make (deploy も実行)
set -eu
source "$(dirname "$0")/common.sh"
"$(dirname "$0")/deploy.sh"
ssh -t "${G1_SSH}" "cd '${G1_REMOTE_DIR}' && mkdir -p build && cd build && cmake .. && make -j\$(nproc)"
