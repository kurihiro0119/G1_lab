cmake_minimum_required(VERSION 3.10)
project(g1_lab CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 公式サンプル (g1_ankle_swing_example など) も一緒にビルドする
option(BUILD_EXAMPLES "Build unitree_sdk2 examples" ON)
add_subdirectory(third_party/unitree_sdk2)

# 自作プログラムは build/bin に出力
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
add_subdirectory(src)
