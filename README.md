# G1_lab — Unitree G1 検証ワークスペース

[TechShare「Unitree G1 SDK」](https://techshare.co.jp/faq/unitree/unitree-g1-sdk.html) の手順（unitree_sdk2 を G1 の開発用PCでビルドして実行する）をもとに、検証用のツール・スクリプト・ドキュメントをまとめたワークスペース。

- 公式: https://support.unitree.com/home/en/G1_developer / https://github.com/unitreerobotics/unitree_sdk2
- 詳しい資料: [docs/architecture.md](docs/architecture.md)（構成・制御レイヤ・関節表） / [docs/safety.md](docs/safety.md)（安全チェックリスト） / [docs/troubleshooting.md](docs/troubleshooting.md)（症状別の対処・エラーコード）

## 全体像

```
 Mac (このリポジトリ)                         G1 開発用PC 192.168.123.164 (Ubuntu 20.04 / aarch64)
 ─────────────────────                         ──────────────────────────────────────────────
 make build  ── rsync ─────────────────────▶  ~/ts_ws/G1_lab  →  cmake && make
 make run P=…  ── ssh ─────────────────────▶  ./bin/<prog> eth0   ──DDS──▶ ロボット
 make log   ◀── rsync logs/*.csv ──────────   g1_state_logger
 make plot  (uv + matplotlib で可視化)
 make docker (Ubuntu20.04 arm64 コンテナでコンパイル確認。実機不要)
```

SDK 同梱ライブラリは Linux 用だけなので、Mac から直接 DDS 通信はできない。プログラムは開発用PCへ送ってビルド・実行する。

## 構成

```
G1_lab/
├── Makefile                 # make help でコマンド一覧
├── CMakeLists.txt           # SDK + 公式サンプル + src/ をまとめてビルド
├── config/robot.env         # IP / ユーザー / 配置先 / IF
├── src/                     # 自作ツール (下表)
│   └── common/g1_joints.hpp #   関節インデックスと名前
├── scripts/                 # 転送・ビルド・実行・ログ取得 (Makefile から呼ぶ)
├── tools/                   # Mac 側の解析ツール (uv プロジェクト)
├── docs/                    # 構成・安全・トラブルシューティング
├── experiments/TEMPLATE.md  # 実験記録のテンプレート
├── logs/                    # 取得したログ (git 管理外)
├── docker/Dockerfile        # コンパイル確認環境
└── third_party/unitree_sdk2 # 公式 SDK
```

## 自作ツール (`src/`)

安全な順に並べている。上の3つは指令を送らない読み取り専用。

| プログラム | 内容 | 例 |
|---|---|---|
| `g1_state_monitor` | 関節角度・角速度・トルク・温度と IMU を 1 秒ごとに表示 | `make run P=g1_state_monitor` |
| `g1_state_logger` | `rt/lowstate` を 500Hz のまま CSV に記録 | `make log D=20 L=idle` |
| `g1_health_check` | 受信レート・傾き・温度・モーターエラーを判定し OK/WARN/FAIL を返す | `make preflight` |
| `g1_audio_led` | TTS 発話・音量・頭部 LED | `make run P=g1_audio_led A='led 0 255 0'` |
| `g1_loco_cli` | 歩行の対話シェル。速度上限 (vx±0.3, vy±0.2, wz±0.5) と 3 秒の自動停止付き。脱力系の操作は yes を入力して確認 | `make run P=g1_loco_cli` |
| `g1_arm_action_cli` | 腕のプリセット動作の一覧・実行・解除 | `make run P=g1_arm_action_cli A='list'` |

公式サンプル（記事の `g1_ankle_swing_example` / `g1_dual_arm_example` など）も同時にビルドされ、同じ `make run` で実行できる。一覧は [docs/architecture.md](docs/architecture.md#公式サンプル早見表-buildthird_partyunitree_sdk2bin)。

## はじめかた

`third_party/unitree_sdk2` は git submodule。clone するときは `--recursive` を付ける（付け忘れたら `git submodule update --init`）:
```bash
git clone --recursive https://github.com/kurihiro0119/G1_lab.git
```

### 0. Mac の準備（初回）
1. 有線LANで G1 と接続し、Mac の有線 IF に `192.168.123.212 / 255.255.255.0` を手動設定
   （システム設定 → ネットワーク → 有線LANアダプタ → 詳細 → TCP/IP → IPv4 を「手入力」）
2. 接続確認と SSH 鍵登録（初期パスワードは記事を参照）
   ```bash
   make net
   ```
   ```bash
   make ssh-key
   ```
3. 実機なしでコンパイルだけ確認したいときは
   ```bash
   make docker
   ```

### 1. ビルド
```bash
make build
```

### 2. 段階的な動作確認（おすすめの順番）
| Step | 内容 | コマンド | ロボットの状態 |
|---|---|---|---|
| 1 | 状態が読めるか | `make run P=g1_state_monitor` | どれでも |
| 2 | 健全性チェック | `make preflight` | どれでも |
| 3 | 指令が届くか (動かない) | `make run P=g1_audio_led A='tts "hello" 0'` | どれでも |
| 4 | 状態の記録と可視化 | `make log D=10 L=idle` → `make plot F=logs/idle_….csv` | どれでも |
| 5 | 高レベル歩行 | `make run P=g1_loco_cli` → `status` / `start` / `move 0.1 0 0 1` | 立位・周囲確認 |
| 6 | 腕プリセット | `make run P=g1_arm_action_cli A='list'` | 歩行制御中 |
| 7 | 記事の低レベルサンプル | `make run P=g1_ankle_swing_example` | **吊り下げ必須** |

`free(): invalid pointer` が出たら sudo で実行する（記事の注意事項）:
```bash
./scripts/remote_run.sh --sudo g1_dual_arm_example
```

### 3. ログ解析（Mac）
```bash
make plot F=logs/idle_20261005_120000.csv
```
- `A='-j L_Knee R_Knee'` で関節を指定（q/dq/tau/temp の4段）、`A='--summary'` で統計だけ、`A='--save'` で PNG 保存
- 実機なしで試すなら `make dummy` → `make plot F=logs/dummy.csv`

## ⚠️ 安全上の重要事項

- `g1_ankle_swing_example` / `g1_dual_arm_example` は起動時に `ReleaseMode()` で **Unitree の運動制御を止める** → 脚が脱力する。**必ず吊り下げて**実行する
- 実験の前に毎回 [docs/safety.md](docs/safety.md) のチェックリストを確認し、`experiments/TEMPLATE.md` をコピーして記録を残す
- `g1_loco_client`（公式）は記事の時点（2025/1/30）で動作未確認。歩行の確認には `g1_loco_cli` を使う

## 自作ツールの追加

1. `src/<名前>.cpp` を作る（関節名は `#include "common/g1_joints.hpp"`）
2. `src/CMakeLists.txt` の `G1LAB_TARGETS` に名前を追加する
3. `make docker` でコンパイル確認 → `make build` → `make run P=<名前>`

## 設定 (`config/robot.env`)

| 変数 | 既定値 | 説明 |
|---|---|---|
| `G1_IP` | 192.168.123.164 | 開発用PC |
| `G1_USER` | unitree | SSH ユーザー |
| `G1_REMOTE_DIR` | /home/unitree/ts_ws/G1_lab | 配置先 (記事の ts_ws に合わせた) |
| `G1_IFACE` | eth0 | 開発用PCで DDS に使う IF |
| `HOST_IP` | 192.168.123.212 | Mac 側の固定 IP |
# G1_lab
