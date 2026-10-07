# トラブルシューティング

| 症状 | 原因と対処 |
|---|---|
| `ping 192.168.123.164` が通らない | Mac の有線 IF に 192.168.123.x/24 が付いていない (`make net`)。Wi-Fi 側と同じサブネットになっていないかも確認 |
| SSH でパスワードを毎回聞かれる | `make ssh-key` で鍵を登録 |
| `free(): invalid pointer` で落ちる | 記事の対処: `sudo` で実行 → `./scripts/remote_run.sh --sudo <prog>` |
| `waiting for rt/lowstate ...` のまま | IF 名が違う (内部PCで `ip -br addr` を確認)。ロボットの電源・起動完了を確認 |
| LocoClient / ArmAction が `3104` などのエラー | サービス呼び出しのタイムアウト。運動制御が起動しきっていない、またはデバッグモード中 |
| ArmAction が `7404` | 歩行制御中 (fsm 500/501/801) でない → `g1_loco_cli` で `start` |
| `cmake` で `Unitree SDK library for the architecture is not found` | Mac で直接 cmake した。内部PCか Docker (`make docker`) でビルドする |
| Docker ビルドが遅い | 初回は apt とビルドで数分かかる。2回目以降は `build-docker/` を再利用 |
| `g1_loco_client` が動かない | 記事時点 (2025/1/30) で動作未確認とされている。`g1_loco_cli` を使う |
| ログのレートが 500Hz より低い | 内部PCの負荷が高い / ネットワーク越しに記録している。内部PC上で記録する (`make log`) |

## SDK のエラーコード (`include/unitree/robot/internal/internal_error.hpp` より)

| code | 意味 | よくある原因 |
|---|---|---|
| 3102 | 送信エラー | DDS 初期化失敗 / IF 名違い |
| 3103 | API 未登録 | クライアントの `Init()` を呼んでいない |
| 3104 | API 呼び出しタイムアウト | サービスが動いていない (デバッグモード中、起動途中) |
| 3105 / 3106 | 応答の不一致 / データエラー | SDK とファームウェアのバージョン差 |
| 3202 | サーバー内部エラー | 現在の状態では受け付けられない指令 |
| 3203 | API 未実装 | ファームウェアがその API に未対応 |
| 3204 | パラメータエラー | 引数の範囲外 |
| 7402 / 7404 | ArmAction: 無効な ID / 無効な FSM | `list` で ID 確認 / 歩行制御中にする |
