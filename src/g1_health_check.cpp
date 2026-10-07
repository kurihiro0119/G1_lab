// 実験前の健全性チェック (読み取り専用)。
// 数秒間 rt/lowstate を受信し、受信レート・IMU・関節温度・モーターエラーを判定して
// 問題があれば終了コード 1 を返す。scripts/preflight.sh から呼ばれる。
//
// usage: ./bin/g1_health_check <network_interface> [sample_sec=3]

#include <unistd.h>

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdio>
#include <iostream>
#include <mutex>
#include <string>

#include <unitree/idl/hg/LowState_.hpp>
#include <unitree/idl/hg/SportModeState_.hpp>
#include <unitree/robot/channel/channel_subscriber.hpp>

#include "common/g1_joints.hpp"

using namespace unitree::robot;
using unitree_hg::msg::dds_::LowState_;
using unitree_hg::msg::dds_::SportModeState_;

// 判定しきい値
constexpr double kMinRateHz = 400.0;     // lowstate は通常 500Hz
constexpr int kTempWarn = 60;            // [°C]
constexpr int kTempError = 75;           // [°C]
constexpr double kMaxTiltRad = 0.6;      // 吊り下げ・直立時の roll/pitch 上限の目安

static std::mutex g_mutex;
static LowState_ g_low;
static std::atomic<uint64_t> g_low_count{0};
static std::array<int, g1lab::kNumJoints> g_max_temp{};
static std::array<uint32_t, g1lab::kNumJoints> g_err_bits{};
static std::atomic<int> g_fsm_id{-1};

static void LowHandler(const void *msg) {
  const auto &s = *static_cast<const LowState_ *>(msg);
  std::lock_guard<std::mutex> lock(g_mutex);
  g_low = s;
  for (int i = 0; i < g1lab::kNumJoints; ++i) {
    g_max_temp[i] = std::max<int>(g_max_temp[i], s.motor_state()[i].temperature()[0]);
    g_err_bits[i] |= s.motor_state()[i].motorstate();
  }
  g_low_count++;
}

static void SportHandler(const void *msg) {
  g_fsm_id = static_cast<int>(static_cast<const SportModeState_ *>(msg)->fsm_id());
}

int main(int argc, char **argv) {
  if (argc < 2) {
    std::cout << "Usage: " << argv[0] << " <network_interface> [sample_sec=3]" << std::endl;
    return 1;
  }
  const int sample_sec = argc > 2 ? std::stoi(argv[2]) : 3;

  ChannelFactory::Instance()->Init(0, argv[1]);
  ChannelSubscriber<LowState_> low_sub("rt/lowstate");
  low_sub.InitChannel(LowHandler, 1);
  ChannelSubscriber<SportModeState_> sport_sub("rt/sportmodestate");
  sport_sub.InitChannel(SportHandler, 1);

  std::cout << "sampling " << sample_sec << "s ..." << std::endl;
  sleep(sample_sec);

  int errors = 0, warnings = 0;
  auto ok = [](const std::string &m) { std::cout << "[ OK ] " << m << std::endl; };
  auto warn = [&](const std::string &m) { std::cout << "[WARN] " << m << std::endl; warnings++; };
  auto fail = [&](const std::string &m) { std::cout << "[FAIL] " << m << std::endl; errors++; };

  const double rate = static_cast<double>(g_low_count.load()) / sample_sec;
  char buf[256];
  if (g_low_count == 0) {
    fail("rt/lowstate を受信できない (IF名 / ロボットの電源 / ネットワークを確認)");
    std::cout << "\nRESULT: FAIL" << std::endl;
    return 1;
  }
  std::snprintf(buf, sizeof(buf), "lowstate rate = %.1f Hz", rate);
  rate >= kMinRateHz ? ok(buf) : warn(std::string(buf) + " (通常 500Hz)");

  std::lock_guard<std::mutex> lock(g_mutex);
  const auto &rpy = g_low.imu_state().rpy();
  std::snprintf(buf, sizeof(buf), "IMU rpy = (%.3f, %.3f, %.3f) rad", rpy[0], rpy[1], rpy[2]);
  (std::fabs(rpy[0]) < kMaxTiltRad && std::fabs(rpy[1]) < kMaxTiltRad) ? ok(buf)
                                                                         : warn(std::string(buf) + " 傾きが大きい");

  std::snprintf(buf, sizeof(buf), "mode_machine = %u, mode_pr = %u", g_low.mode_machine(), g_low.mode_pr());
  ok(buf);

  const int fsm = g_fsm_id.load();
  if (fsm < 0)
    std::cout << "[INFO] rt/sportmodestate 未受信 (デバッグモード中は正常)" << std::endl;
  else
    std::cout << "[INFO] loco fsm_id = " << fsm << " (0:ZeroTorque 1:Damp 500/501:歩行制御 等)" << std::endl;

  int hottest = 0;
  for (int i = 0; i < g1lab::kNumJoints; ++i) {
    if (g_max_temp[i] > g_max_temp[hottest]) hottest = i;
    if (g_max_temp[i] >= kTempError) {
      std::snprintf(buf, sizeof(buf), "%s temp %d°C >= %d°C", g1lab::kJointNames[i], g_max_temp[i], kTempError);
      fail(buf);
    } else if (g_max_temp[i] >= kTempWarn) {
      std::snprintf(buf, sizeof(buf), "%s temp %d°C >= %d°C", g1lab::kJointNames[i], g_max_temp[i], kTempWarn);
      warn(buf);
    }
    if (g_err_bits[i] != 0) {
      std::snprintf(buf, sizeof(buf), "%s motorstate=0x%08x (エラービット)", g1lab::kJointNames[i], g_err_bits[i]);
      warn(buf);
    }
  }
  std::snprintf(buf, sizeof(buf), "max motor temp = %d°C (%s)", g_max_temp[hottest], g1lab::kJointNames[hottest]);
  ok(buf);

  std::cout << "\nRESULT: " << (errors ? "FAIL" : warnings ? "WARN" : "OK") << "  (errors=" << errors
            << ", warnings=" << warnings << ")" << std::endl;
  return errors ? 1 : 0;
}
