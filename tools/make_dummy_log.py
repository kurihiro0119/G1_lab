"""実機なしで plot_log.py を試すためのダミー CSV を生成する。

usage: uv run --project tools tools/make_dummy_log.py [out=logs/dummy.csv]
"""

import math
import sys

JOINTS = ["L_HipPitch", "L_HipRoll", "L_HipYaw", "L_Knee", "L_AnklePitch", "L_AnkleRoll",
          "R_HipPitch", "R_HipRoll", "R_HipYaw", "R_Knee", "R_AnklePitch", "R_AnkleRoll",
          "WaistYaw", "WaistRoll", "WaistPitch",
          "L_ShoulderPitch", "L_ShoulderRoll", "L_ShoulderYaw", "L_Elbow",
          "L_WristRoll", "L_WristPitch", "L_WristYaw",
          "R_ShoulderPitch", "R_ShoulderRoll", "R_ShoulderYaw", "R_Elbow",
          "R_WristRoll", "R_WristPitch", "R_WristYaw"]

out = sys.argv[1] if len(sys.argv) > 1 else "logs/dummy.csv"
with open(out, "w") as f:
    cols = ["t", "tick", "mode_machine", "roll", "pitch", "yaw", "gx", "gy", "gz", "ax", "ay", "az"]
    for field in ("q", "dq", "tau", "temp"):
        cols += [f"{field}_{j}" for j in JOINTS]
    f.write(",".join(cols) + "\n")
    for k in range(2500):  # 5s @ 500Hz
        t = k * 0.002
        row = [t, k * 2, 5, 0.01 * math.sin(t), 0.02 * math.sin(2 * t), 0.0, 0, 0, 0, 0, 0, 9.8]
        row += [0.3 * math.sin(2 * math.pi * 0.5 * t + i * 0.2) for i in range(29)]
        row += [0.3 * math.pi * math.cos(2 * math.pi * 0.5 * t + i * 0.2) for i in range(29)]
        row += [2.0 * math.sin(2 * math.pi * 0.5 * t + i * 0.2) for i in range(29)]
        row += [35 + i % 5 for i in range(29)]
        f.write(",".join(f"{v:.5g}" for v in row) + "\n")
print(f"wrote {out}")
