// 音声 (TTS / 音量) と頭部 LED の操作。動かない機能なので最初の動作確認に向く。
//
// usage: ./bin/g1_audio_led <iface> tts "<text>" [speaker_id=0]
//        ./bin/g1_audio_led <iface> volume [0-100]     (引数なしで現在値を表示)
//        ./bin/g1_audio_led <iface> led <r> <g> <b>    (0-255)

#include <unistd.h>

#include <iostream>
#include <string>

#include <unitree/robot/g1/audio/g1_audio_client.hpp>

using unitree::robot::ChannelFactory;
using unitree::robot::g1::AudioClient;

static int Usage(const char *prog) {
  std::cout << "Usage: " << prog << " <iface> tts \"<text>\" [speaker_id] | volume [0-100] | led <r> <g> <b>"
            << std::endl;
  return 1;
}

int main(int argc, char **argv) {
  if (argc < 3) return Usage(argv[0]);
  const std::string sub = argv[2];

  ChannelFactory::Instance()->Init(0, argv[1]);
  AudioClient client;
  client.Init();
  client.SetTimeout(10.f);

  int32_t ret = 0;
  if (sub == "tts" && argc > 3) {
    int speaker = argc > 4 ? std::stoi(argv[4]) : 0;
    ret = client.TtsMaker(argv[3], speaker);
    sleep(1);  // 再生開始を待つ
  } else if (sub == "volume") {
    if (argc > 3) {
      ret = client.SetVolume(static_cast<uint8_t>(std::stoi(argv[3])));
    } else {
      uint8_t vol = 0;
      ret = client.GetVolume(vol);
      std::cout << "volume = " << int(vol) << std::endl;
    }
  } else if (sub == "led" && argc > 5) {
    ret = client.LedControl(std::stoi(argv[3]), std::stoi(argv[4]), std::stoi(argv[5]));
  } else {
    return Usage(argv[0]);
  }
  std::cout << (ret == 0 ? "ok" : "error code " + std::to_string(ret)) << std::endl;
  return ret == 0 ? 0 : 1;
}
