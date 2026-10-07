// 腕のプリセット動作 (G1ArmActionClient) を実行する。
// 動作は歩行制御中 (fsm_id 500/501/801) でのみ受け付けられる。
//
// usage: ./bin/g1_arm_action_cli <iface> list
//        ./bin/g1_arm_action_cli <iface> id <action_id>     (99 = 解除)
//        ./bin/g1_arm_action_cli <iface> name <teach_action_name>
//        ./bin/g1_arm_action_cli <iface> release

#include <iostream>
#include <string>

#include <unitree/robot/g1/arm/g1_arm_action_client.hpp>
#include <unitree/robot/g1/arm/g1_arm_action_error.hpp>

using unitree::robot::ChannelFactory;
using unitree::robot::g1::G1ArmActionClient;
using namespace unitree::robot::g1;  // エラーコード定数

static int Usage(const char *prog) {
  std::cout << "Usage: " << prog << " <iface> list | id <action_id> | name <teach_name> | release" << std::endl;
  return 1;
}

int main(int argc, char **argv) {
  if (argc < 3) return Usage(argv[0]);
  const std::string sub = argv[2];

  ChannelFactory::Instance()->Init(0, argv[1]);
  G1ArmActionClient client;
  client.Init();
  client.SetTimeout(10.f);

  int32_t ret = 0;
  if (sub == "list") {
    std::string data;
    ret = client.GetActionList(data);
    std::cout << data << std::endl;
  } else if (sub == "id" && argc > 3) {
    ret = client.ExecuteAction(std::stoi(argv[3]));
  } else if (sub == "name" && argc > 3) {
    ret = client.ExecuteAction(std::string(argv[3]));
  } else if (sub == "release") {
    ret = client.ExecuteAction(99);
  } else {
    return Usage(argv[0]);
  }

  if (ret == UT_ROBOT_ARM_ACTION_ERR_INVALID_FSM_ID)
    std::cerr << "歩行制御中 (fsm_id 500/501/801) でのみ実行できます。g1_loco_cli で start してください。" << std::endl;
  else if (ret == UT_ROBOT_ARM_ACTION_ERR_INVALID_ACTION_ID)
    std::cerr << "無効な action id です。list で確認してください。" << std::endl;
  else if (ret != 0)
    std::cerr << "error code " << ret << std::endl;
  else
    std::cout << "ok" << std::endl;
  return ret == 0 ? 0 : 1;
}
