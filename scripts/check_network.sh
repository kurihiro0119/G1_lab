#!/usr/bin/env bash
# Mac と G1 の有線接続を確認する
set -u
source "$(dirname "$0")/common.sh"

echo "== Mac 側の 192.168.123.x アドレス =="
if ifconfig | grep -q "inet 192\.168\.123\."; then
  ifconfig | grep -B6 "inet 192\.168\.123\." | grep -E "^[a-z]|inet 192"
else
  echo "見つかりません。有線LANのIFに ${HOST_IP}/24 を設定してください。"
  echo "  例: networksetup -listallhardwareports でサービス名を確認し"
  echo "      sudo networksetup -setmanual \"<USB 10/100/1000 LAN 等>\" ${HOST_IP} 255.255.255.0"
fi

echo
echo "== ping ${G1_IP} (内部PC) =="
ping -c 2 -t 3 "${G1_IP}" >/dev/null 2>&1 && echo "OK" || echo "NG"

echo
echo "== ssh ${G1_SSH} =="
if ssh -o BatchMode=yes -o ConnectTimeout=5 "${G1_SSH}" "uname -a; lsb_release -ds 2>/dev/null; ip -br addr | grep -E '^(eth|wlan)'" 2>/dev/null; then
  echo "OK"
else
  echo "鍵認証で接続できません (scripts/setup_ssh_key.sh を実行。初期パスワードは記事参照)"
fi
