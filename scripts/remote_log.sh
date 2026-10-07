#!/usr/bin/env bash
# 内部PCで rt/lowstate を記録して Mac に取り寄せる
#   usage: scripts/remote_log.sh [duration_sec=10] [label]
#   例:    scripts/remote_log.sh 20 walk_forward
set -eu
source "$(dirname "$0")/common.sh"
DUR="${1:-10}"
LABEL="${2:-lowstate}"
NAME="${LABEL}_$(date +%Y%m%d_%H%M%S).csv"
"$(dirname "$0")/remote_run.sh" g1_state_logger "${DUR}" "../logs/${NAME}"
"$(dirname "$0")/fetch_logs.sh"
echo "local: logs/${NAME}"
echo "plot : uv run --project tools tools/plot_log.py logs/${NAME}"
