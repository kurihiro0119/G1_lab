# 自作の検証プログラム。追加するときは G1LAB_TARGETS に名前を足して <名前>.cpp を置く
set(G1LAB_TARGETS
  g1_state_monitor     # 読み取り専用: 状態表示
  g1_state_logger      # 読み取り専用: CSV 記録
  g1_health_check      # 読み取り専用: 実験前チェック
  g1_audio_led         # 音声 / LED
  g1_loco_cli          # 高レベル歩行 (安全上限付き)
  g1_arm_action_cli    # 腕プリセット動作
)

foreach(target ${G1LAB_TARGETS})
  add_executable(${target} ${target}.cpp)
  target_include_directories(${target} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
  target_link_libraries(${target} unitree_sdk2)
endforeach()
