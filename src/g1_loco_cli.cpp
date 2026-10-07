// 高レベル運動 (LocoClient) を対話的に操作する CLI。
// - 速度指令は安全上限でクリップし、必ず duration 付きで送る (指令が途切れれば自動停止)
// - 脱力系 (damp / zero) は "yes" 入力で確認してから実行
// - 終了時・Ctrl+C 時は StopMove を送る
//
// usage: ./bin/g1_loco_cli <network_interface>

#include <algorithm>
#include <csignal>
#include <iostream>
#include <sstream>
#include <string>

#include <unitree/robot/g1/loco/g1_loco_client.hpp>

using unitree::robot::ChannelFactory;
using unitree::robot::g1::LocoClient;

// 検証用の安全上限 (必要に応じて変更)
constexpr float kMaxVx = 0.3f;    // [m/s]
constexpr float kMaxVy = 0.2f;    // [m/s]
constexpr float kMaxWz = 0.5f;    // [rad/s]
constexpr float kMaxDur = 3.0f;   // [s]

static LocoClient *g_client = nullptr;

static void OnSignal(int) {
  if (g_client) g_client->StopMove();
  std::cout << "\nStopMove sent. bye." << std::endl;
  std::_Exit(0);
}

static bool Confirm(const std::string &what) {
  std::cout << "⚠️  " << what << "\n   本当に実行しますか? (yes/no): " << std::flush;
  std::string ans;
  std::getline(std::cin, ans);
  return ans == "yes";
}

static void Report(const char *name, int32_t ret) {
  std::cout << (ret == 0 ? "  ok: " : "  error: ") << name;
  if (ret != 0) std::cout << " (code " << ret << ")";
  std::cout << std::endl;
}

static void Help() {
  std::cout << R"(commands:
  status                 fsm_id / fsm_mode / balance_mode / stand_height を表示
  start                  歩行制御を開始 (fsm 500)
  standup | sit | squat  立ち上がり / 座る / しゃがむ
  high | low             立ち高さ 最大 / 最小
  move <vx> <vy> <wz> [dur]   速度指令 (上限: vx±0.3 vy±0.2 wz±0.5, dur<=3s)
  stop                   停止 (速度0)
  wave | shake           手を振る / 握手
  damp                   ダンピング (要確認: 立位なら崩れる)
  zero                   ゼロトルク (要確認: 完全に脱力する)
  help | quit
)";
}

int main(int argc, char **argv) {
  if (argc < 2) {
    std::cout << "Usage: " << argv[0] << " <network_interface>" << std::endl;
    return 1;
  }
  ChannelFactory::Instance()->Init(0, argv[1]);
  LocoClient client;
  client.Init();
  client.SetTimeout(10.f);
  g_client = &client;
  std::signal(SIGINT, OnSignal);

  std::cout << "G1 loco CLI. ロボット周囲 2m に人・物が無いこと、リモコンで非常停止できることを確認してください。\n";
  Help();

  std::string line;
  while (std::cout << "loco> " << std::flush, std::getline(std::cin, line)) {
    std::istringstream is(line);
    std::string cmd;
    if (!(is >> cmd)) continue;

    if (cmd == "quit" || cmd == "exit") break;
    else if (cmd == "help") Help();
    else if (cmd == "status") {
      int fsm_id = -1, fsm_mode = -1, balance = -1;
      float height = 0.f;
      client.GetFsmId(fsm_id);
      client.GetFsmMode(fsm_mode);
      client.GetBalanceMode(balance);
      client.GetStandHeight(height);
      std::cout << "  fsm_id=" << fsm_id << " fsm_mode=" << fsm_mode << " balance_mode=" << balance
                << " stand_height=" << height << std::endl;
    }
    else if (cmd == "start") Report("Start", client.Start());
    else if (cmd == "standup") Report("StandUp", client.StandUp());
    else if (cmd == "sit") Report("Sit", client.Sit());
    else if (cmd == "squat") Report("Squat", client.Squat());
    else if (cmd == "high") Report("HighStand", client.HighStand());
    else if (cmd == "low") Report("LowStand", client.LowStand());
    else if (cmd == "stop") Report("StopMove", client.StopMove());
    else if (cmd == "wave") Report("WaveHand", client.WaveHand());
    else if (cmd == "shake") Report("ShakeHand", client.ShakeHand());
    else if (cmd == "move") {
      float vx, vy, wz, dur = 1.f;
      if (!(is >> vx >> vy >> wz)) { std::cout << "  usage: move <vx> <vy> <wz> [dur]" << std::endl; continue; }
      is >> dur;
      float cvx = std::clamp(vx, -kMaxVx, kMaxVx), cvy = std::clamp(vy, -kMaxVy, kMaxVy);
      float cwz = std::clamp(wz, -kMaxWz, kMaxWz), cdur = std::clamp(dur, 0.1f, kMaxDur);
      if (cvx != vx || cvy != vy || cwz != wz || cdur != dur) std::cout << "  (安全上限でクリップしました)" << std::endl;
      std::cout << "  move vx=" << cvx << " vy=" << cvy << " wz=" << cwz << " dur=" << cdur << "s" << std::endl;
      Report("SetVelocity", client.SetVelocity(cvx, cvy, cwz, cdur));
    }
    else if (cmd == "damp") { if (Confirm("ダンピングへ移行します (立位では崩れ落ちます)")) Report("Damp", client.Damp()); }
    else if (cmd == "zero") { if (Confirm("ゼロトルクにします (完全に脱力します)")) Report("ZeroTorque", client.ZeroTorque()); }
    else std::cout << "  unknown command: " << cmd << " (help で一覧)" << std::endl;
  }
  client.StopMove();
  return 0;
}
