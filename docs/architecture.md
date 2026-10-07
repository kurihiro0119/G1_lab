# G1 システム構成と制御レイヤ

> SDK (`third_party/unitree_sdk2`) のソースと [公式ドキュメント](https://support.unitree.com/home/en/G1_developer) をもとに整理。
> 「要確認」と書いた箇所はファームウェアのバージョンで変わる可能性があるので、公式ドキュメントで確認すること。

## 計算機とネットワーク

```
 Mac (192.168.123.212)
   │ 有線LAN (192.168.123.0/24)
   ▼
 G1 内部スイッチ
   ├─ 運動制御PC   192.168.123.161  … Unitree の歩行制御が動く (ユーザー非公開)
   └─ 開発用PC     192.168.123.164  … Jetson Orin / Ubuntu 20.04 / aarch64 (user: unitree)
                                       ← このワークスペースを置いてビルド・実行する場所
```

- 通信は DDS (CycloneDDS)。`ChannelFactory::Instance()->Init(0, "<IF名>")` の IF は、**192.168.123.x が付いている IF** を指定する（開発用PCでは `eth0`）
- 同梱ライブラリが Linux (x86_64 / aarch64) 用だけなので、Mac から直接 DDS には参加できない

## 制御レイヤ (上ほど安全)

| レイヤ | 使うもの | 主なトピック / サービス | このワークスペースのツール | 歩行制御 |
|---|---|---|---|---|
| 状態取得 | `ChannelSubscriber` | `rt/lowstate` (500Hz), `rt/sportmodestate` | `g1_state_monitor` / `g1_state_logger` / `g1_health_check` | 影響なし |
| 音声・LED | `g1::AudioClient` | サービス `voice` | `g1_audio_led` | 影響なし |
| 高レベル歩行 | `g1::LocoClient` | サービス `sport` (FSM / 速度指令) | `g1_loco_cli` | Unitree 制御のまま |
| 腕プリセット | `g1::G1ArmActionClient` | サービス `arm` | `g1_arm_action_cli` | 歩行中 (fsm 500/501/801) のみ |
| 腕の中レベル制御 | `ChannelPublisher<LowCmd_>` | `rt/arm_sdk` | 公式 `g1_arm7_sdk_dds_example` | 下半身は Unitree 制御のまま、上半身を上書き |
| 低レベル全身制御 | `ChannelPublisher<LowCmd_>` | `rt/lowcmd` | 公式 `g1_ankle_swing_example` / `g1_dual_arm_example` | **止める必要あり** (要: 吊り下げ) |

### rt/arm_sdk (中レベル) の要点
- `LowCmd_` を `rt/arm_sdk` に送ると、上半身だけ自分の指令で上書きできる
- 未使用関節 (index 29 = `kNotUsedJoint`) の `q` を **重み (0.0〜1.0)** として使う。0→1 にゆっくり上げて乗っ取り、終了時は 1→0 にゆっくり下げて返す（公式例は 0.2/s）

### rt/lowcmd (低レベル) の要点
- 送る前に Unitree の運動制御を止める必要がある。公式例は `MotionSwitcherClient::ReleaseMode()` を使って止めている
  - リモコンでデバッグモードに入る方法（ダンピング状態で L2+R2）もある（要確認）
- `mode_pr` (0: Pitch/Roll 直列, 1: A/B 並列) と `mode_machine` は `rt/lowstate` の値をそのまま返す
- CRC32 を `crc` に入れないと受け付けられない (公式例の `Crc32Core`)
- 指令周期は 2ms (500Hz) が基本

## LocoClient の FSM ID (SDK ヘッダより)

| ID | 意味 | API |
|---|---|---|
| 0 | ゼロトルク (完全脱力) | `ZeroTorque()` |
| 1 | ダンピング | `Damp()` |
| 2 | しゃがむ | `Squat()` |
| 3 | 座る | `Sit()` |
| 4 | 立ち上がり (ロック立ち) | `StandUp()` |
| 500 | 歩行制御開始 | `Start()` |

速度指令 `SetVelocity(vx, vy, wz, duration)` は duration 経過で止まる。`Move()` は連続移動モード次第で止まらないことがあるので、検証では `SetVelocity` を使う。

## 関節インデックス (29DoF)

| idx | 関節 | idx | 関節 | idx | 関節 |
|---|---|---|---|---|---|
| 0 | L_HipPitch | 10 | R_AnklePitch (B) | 20 | L_WristPitch * |
| 1 | L_HipRoll | 11 | R_AnkleRoll (A) | 21 | L_WristYaw * |
| 2 | L_HipYaw | 12 | WaistYaw | 22 | R_ShoulderPitch |
| 3 | L_Knee | 13 | WaistRoll (A) ** | 23 | R_ShoulderRoll |
| 4 | L_AnklePitch (B) | 14 | WaistPitch (B) ** | 24 | R_ShoulderYaw |
| 5 | L_AnkleRoll (A) | 15 | L_ShoulderPitch | 25 | R_Elbow |
| 6 | R_HipPitch | 16 | L_ShoulderRoll | 26 | R_WristRoll |
| 7 | R_HipRoll | 17 | L_ShoulderYaw | 27 | R_WristPitch * |
| 8 | R_HipYaw | 18 | L_Elbow | 28 | R_WristYaw * |
| 9 | R_Knee | 19 | L_WristRoll | 29 | (arm_sdk の重み) |

\* 23DoF モデルでは無効 / \*\* 腰ロックモデルでは無効。定義は `src/common/g1_joints.hpp`。

## 公式サンプル早見表 (`build/third_party/unitree_sdk2/bin/`)

| サンプル | レイヤ | 内容 |
|---|---|---|
| `g1_loco_client` | 高 | `--get_fsm_id`, `--set_velocity="0.2 0 0 1"` などのオプションで LocoClient を呼ぶ |
| `g1_arm_action_example` | 高 | `-l` で一覧、`-i <id>` で実行 |
| `g1_audio_client_example` | 高 | TTS / ASR / WAV 再生 |
| `g1_arm5_sdk_dds_example` / `g1_arm7_sdk_dds_example` | 中 | rt/arm_sdk で腕を動かす |
| `g1_ankle_swing_example` | 低 | 足首をスイング (記事のサンプル) |
| `g1_dual_arm_example` | 低 | 両腕制御 (記事のサンプル) |
| `g1_dex3_example` | 低 | Dex3 ハンド |
