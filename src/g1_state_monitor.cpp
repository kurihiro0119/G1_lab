// G1 の rt/lowstate を購読して表示するだけの読み取り専用プログラム。
// LowCmd を送信せず、モーション制御サービスも停止しないので、
// SDK 疎通確認の最初の一歩として安全に実行できる。
//
// usage: ./bin/g1_state_monitor <network_interface>   (例: eth0)

#include <unistd.h>

#include <atomic>
#include <cstdio>
#include <iostream>
#include <mutex>

#include <unitree/idl/hg/LowState_.hpp>
#include <unitree/robot/channel/channel_subscriber.hpp>

#include "common/g1_joints.hpp"

using namespace unitree::robot;
using unitree_hg::msg::dds_::LowState_;
using g1lab::kJointNames;


static std::mutex g_mutex;
static LowState_ g_state;
static std::atomic<uint64_t> g_count{0};

static void LowStateHandler(const void *msg) {
  std::lock_guard<std::mutex> lock(g_mutex);
  g_state = *static_cast<const LowState_ *>(msg);
  g_count++;
}

int main(int argc, char **argv) {
  if (argc < 2) {
    std::cout << "Usage: " << argv[0] << " <network_interface>" << std::endl;
    return 1;
  }

  ChannelFactory::Instance()->Init(0, argv[1]);
  ChannelSubscriber<LowState_> subscriber("rt/lowstate");
  subscriber.InitChannel(LowStateHandler, 1);

  std::cout << "Subscribing rt/lowstate on " << argv[1] << " ... (Ctrl+C to quit)" << std::endl;

  uint64_t last_count = 0;
  while (true) {
    sleep(1);
    uint64_t count = g_count.load();
    if (count == 0) {
      std::cout << "waiting for rt/lowstate ... (interface / subnet を確認)" << std::endl;
      continue;
    }

    LowState_ s;
    {
      std::lock_guard<std::mutex> lock(g_mutex);
      s = g_state;
    }

    const auto &imu = s.imu_state();
    std::printf("\n=== tick=%u  rate=%luHz  mode_machine=%u ===\n", s.tick(),
                static_cast<unsigned long>(count - last_count), s.mode_machine());
    std::printf("IMU rpy[rad]: % .3f % .3f % .3f\n", imu.rpy()[0], imu.rpy()[1], imu.rpy()[2]);
    std::printf("%-16s %9s %9s %9s %5s\n", "joint", "q[rad]", "dq[rad/s]", "tau[Nm]", "temp");
    for (int i = 0; i < g1lab::kNumJoints; ++i) {
      const auto &m = s.motor_state()[i];
      std::printf("%-16s % 9.3f % 9.3f % 9.3f %5d\n", kJointNames[i], m.q(), m.dq(),
                  m.tau_est(), m.temperature()[0]);
    }
    last_count = count;
  }
  return 0;
}
