// G1 (29DoF) の関節インデックスと名前。
// 23DoF モデルでは WaistRoll/WaistPitch/WristPitch/WristYaw は無効。
#pragma once

#include <array>

namespace g1lab {

constexpr int kNumJoints = 29;

enum JointIndex {
  LeftHipPitch = 0, LeftHipRoll, LeftHipYaw, LeftKnee, LeftAnklePitch, LeftAnkleRoll,
  RightHipPitch = 6, RightHipRoll, RightHipYaw, RightKnee, RightAnklePitch, RightAnkleRoll,
  WaistYaw = 12, WaistRoll, WaistPitch,
  LeftShoulderPitch = 15, LeftShoulderRoll, LeftShoulderYaw, LeftElbow,
  LeftWristRoll, LeftWristPitch, LeftWristYaw,
  RightShoulderPitch = 22, RightShoulderRoll, RightShoulderYaw, RightElbow,
  RightWristRoll, RightWristPitch, RightWristYaw,
};

constexpr std::array<const char *, kNumJoints> kJointNames = {
    "L_HipPitch", "L_HipRoll", "L_HipYaw", "L_Knee", "L_AnklePitch", "L_AnkleRoll",
    "R_HipPitch", "R_HipRoll", "R_HipYaw", "R_Knee", "R_AnklePitch", "R_AnkleRoll",
    "WaistYaw", "WaistRoll", "WaistPitch",
    "L_ShoulderPitch", "L_ShoulderRoll", "L_ShoulderYaw", "L_Elbow",
    "L_WristRoll", "L_WristPitch", "L_WristYaw",
    "R_ShoulderPitch", "R_ShoulderRoll", "R_ShoulderYaw", "R_Elbow",
    "R_WristRoll", "R_WristPitch", "R_WristYaw"};

}  // namespace g1lab
