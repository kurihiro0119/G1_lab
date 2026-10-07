"""g1_state_logger の CSV を可視化する。

usage:
  uv run --project tools tools/plot_log.py logs/xxx.csv                 # 概要 (IMU + 全関節 q)
  uv run --project tools tools/plot_log.py logs/xxx.csv -j L_Knee R_Knee # 指定関節の q/dq/tau/temp
  uv run --project tools tools/plot_log.py logs/xxx.csv --summary        # 統計だけ表示
  オプション --save で PNG を CSV と同じ場所に保存 (画面表示しない)
"""

from __future__ import annotations

import argparse
from pathlib import Path

import matplotlib.pyplot as plt
import pandas as pd

GROUPS = {
    "left leg": ["L_HipPitch", "L_HipRoll", "L_HipYaw", "L_Knee", "L_AnklePitch", "L_AnkleRoll"],
    "right leg": ["R_HipPitch", "R_HipRoll", "R_HipYaw", "R_Knee", "R_AnklePitch", "R_AnkleRoll"],
    "waist": ["WaistYaw", "WaistRoll", "WaistPitch"],
    "left arm": ["L_ShoulderPitch", "L_ShoulderRoll", "L_ShoulderYaw", "L_Elbow",
                 "L_WristRoll", "L_WristPitch", "L_WristYaw"],
    "right arm": ["R_ShoulderPitch", "R_ShoulderRoll", "R_ShoulderYaw", "R_Elbow",
                  "R_WristRoll", "R_WristPitch", "R_WristYaw"],
}


def summary(df: pd.DataFrame) -> None:
    dt = df["t"].diff().dropna()
    print(f"rows={len(df)}  duration={df['t'].iloc[-1]:.2f}s  rate={1 / dt.mean():.1f}Hz  "
          f"max_gap={dt.max() * 1000:.1f}ms")
    ticks = df["tick"].diff().dropna()
    print(f"tick step: median={ticks.median():.0f}  dropped(step>median)={(ticks > ticks.median()).sum()}")
    joints = [c[2:] for c in df.columns if c.startswith("q_")]
    rows = []
    for j in joints:
        rows.append({
            "joint": j,
            "q_min": df[f"q_{j}"].min(), "q_max": df[f"q_{j}"].max(),
            "|dq|max": df[f"dq_{j}"].abs().max(),
            "|tau|max": df[f"tau_{j}"].abs().max(),
            "temp_max": df[f"temp_{j}"].max(),
        })
    print(pd.DataFrame(rows).set_index("joint").round(3).to_string())


def plot_overview(df: pd.DataFrame, title: str):
    fig, axes = plt.subplots(len(GROUPS) + 1, 1, figsize=(12, 14), sharex=True)
    for k in ("roll", "pitch", "yaw"):
        axes[0].plot(df["t"], df[k], label=k)
    axes[0].set_ylabel("IMU [rad]")
    axes[0].legend(loc="upper right", ncol=3, fontsize=8)
    for ax, (name, joints) in zip(axes[1:], GROUPS.items()):
        for j in joints:
            ax.plot(df["t"], df[f"q_{j}"], label=j)
        ax.set_ylabel(f"{name}\nq [rad]")
        ax.legend(loc="upper right", ncol=4, fontsize=7)
    axes[-1].set_xlabel("t [s]")
    fig.suptitle(title)
    fig.tight_layout()
    return fig


def plot_joints(df: pd.DataFrame, joints: list[str], title: str):
    fields = [("q", "q [rad]"), ("dq", "dq [rad/s]"), ("tau", "tau [Nm]"), ("temp", "temp [°C]")]
    fig, axes = plt.subplots(len(fields), 1, figsize=(12, 10), sharex=True)
    for ax, (f, label) in zip(axes, fields):
        for j in joints:
            ax.plot(df["t"], df[f"{f}_{j}"], label=j)
        ax.set_ylabel(label)
        ax.legend(loc="upper right", fontsize=8)
    axes[-1].set_xlabel("t [s]")
    fig.suptitle(title)
    fig.tight_layout()
    return fig


def main() -> None:
    p = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument("csv", type=Path)
    p.add_argument("-j", "--joints", nargs="+", help="joint names (e.g. L_Knee R_Knee)")
    p.add_argument("--summary", action="store_true", help="print statistics only")
    p.add_argument("--save", action="store_true", help="save PNG next to the CSV instead of showing")
    args = p.parse_args()

    df = pd.read_csv(args.csv)
    summary(df)
    if args.summary:
        return

    if args.joints:
        unknown = [j for j in args.joints if f"q_{j}" not in df.columns]
        if unknown:
            raise SystemExit(f"unknown joints: {unknown}")
        fig = plot_joints(df, args.joints, args.csv.name)
        suffix = "_" + "_".join(args.joints)
    else:
        fig = plot_overview(df, args.csv.name)
        suffix = "_overview"

    if args.save:
        out = args.csv.with_name(args.csv.stem + suffix + ".png")
        fig.savefig(out, dpi=120)
        print(f"saved: {out}")
    else:
        plt.show()


if __name__ == "__main__":
    main()
