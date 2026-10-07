# 各スクリプト共通: 設定読み込み
ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
# shellcheck source=../config/robot.env
source "${ROOT_DIR}/config/robot.env"
G1_SSH="${G1_USER}@${G1_IP}"
