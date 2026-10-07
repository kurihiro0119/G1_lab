#!/usr/bin/env bash
# Mac の公開鍵を G1 内部PCに登録 (パスワード入力は1回だけ)
set -eu
source "$(dirname "$0")/common.sh"
[ -f ~/.ssh/id_ed25519.pub ] || ssh-keygen -t ed25519 -N "" -f ~/.ssh/id_ed25519
ssh-copy-id -i ~/.ssh/id_ed25519.pub "${G1_SSH}"
