// rt/lowstate を受信レートのまま CSV に記録する (読み取り専用)。
// 記録した CSV は scripts/fetch_logs.sh で Mac に取り寄せ、tools/plot_log.py で可視化する。
//
// usage: ./bin/g1_state_logger <network_interface> [duration_sec=10] [out.csv]

#include <unistd.h>

#include <atomic>
#include <chrono>
#include <cstdio>
#include <ctime>
#include <fstream>
#include <iostream>
#include <mutex>
#include <string>

#include <unitree/idl/hg/LowState_.hpp>
#include <unitree/robot/channel/channel_subscriber.hpp>

#include "common/g1_joints.hpp"

using namespace unitree::robot;
using unitree_hg::msg::dds_::LowState_;
using Clock = std::chrono::steady_clock;

static std::mutex g_mutex;
static std::ofstream g_out;
static Clock::time_point g_t0;
static std::atomic<uint64_t> g_rows{0};

// build/ から実行される前提で、ワークスペース直下の logs/ に保存する
static std::string DefaultPath() {
  char buf[64];
  std::time_t now = std::time(nullptr);
  std::strftime(buf, sizeof(buf), "../logs/lowstate_%Y%m%d_%H%M%S.csv", std::localtime(&now));
  return buf;
}

static void WriteHeader() {
  g_out << "t,tick,mode_machine,roll,pitch,yaw,gx,gy,gz,ax,ay,az";
  for (const char *field : {"q", "dq", "tau", "temp"})
    for (int i = 0; i < g1lab::kNumJoints; ++i) g_out << ',' << field << '_' << g1lab::kJointNames[i];
  g_out << '\n';
}

static void Handler(const void *msg) {
  const auto &s = *static_cast<const LowState_ *>(msg);
  const auto &imu = s.imu_state();
  double t = std::chrono::duration<double>(Clock::now() - g_t0).count();

  std::lock_guard<std::mutex> lock(g_mutex);
  g_out << t << ',' << s.tick() << ',' << int(s.mode_machine());
  for (float v : imu.rpy()) g_out << ',' << v;
  for (float v : imu.gyroscope()) g_out << ',' << v;
  for (float v : imu.accelerometer()) g_out << ',' << v;
  for (int i = 0; i < g1lab::kNumJoints; ++i) g_out << ',' << s.motor_state()[i].q();
  for (int i = 0; i < g1lab::kNumJoints; ++i) g_out << ',' << s.motor_state()[i].dq();
  for (int i = 0; i < g1lab::kNumJoints; ++i) g_out << ',' << s.motor_state()[i].tau_est();
  for (int i = 0; i < g1lab::kNumJoints; ++i) g_out << ',' << s.motor_state()[i].temperature()[0];
  g_out << '\n';
  g_rows++;
}

int main(int argc, char **argv) {
  if (argc < 2) {
    std::cout << "Usage: " << argv[0] << " <network_interface> [duration_sec=10] [out.csv]" << std::endl;
    return 1;
  }
  const double duration = argc > 2 ? std::stod(argv[2]) : 10.0;
  const std::string path = argc > 3 ? argv[3] : DefaultPath();

  g_out.open(path);
  if (!g_out) {
    std::cerr << "cannot open " << path << " (logs/ ディレクトリはありますか?)" << std::endl;
    return 1;
  }
  WriteHeader();

  ChannelFactory::Instance()->Init(0, argv[1]);
  ChannelSubscriber<LowState_> subscriber("rt/lowstate");
  g_t0 = Clock::now();
  subscriber.InitChannel(Handler, 1);

  std::cout << "recording " << duration << "s -> " << path << std::endl;
  for (int sec = 1; sec <= static_cast<int>(duration + 0.999); ++sec) {
    sleep(1);
    std::cout << "  " << sec << "s  rows=" << g_rows.load() << std::endl;
  }
  subscriber.CloseChannel();

  std::lock_guard<std::mutex> lock(g_mutex);
  g_out.close();
  std::cout << "done: " << g_rows.load() << " rows (" << g_rows.load() / duration << " Hz)" << std::endl;
  return g_rows.load() > 0 ? 0 : 2;
}
