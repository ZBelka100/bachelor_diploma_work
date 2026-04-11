#!/usr/bin/env python3

from __future__ import annotations

import argparse
from pathlib import Path

import matplotlib
matplotlib.use("Agg")

import matplotlib.pyplot as plt
import pandas as pd


METHOD_COLORS = {
    "fwht": "tab:blue",
    "fwht_ffht": "tab:green",
    "fft_fftw": "tab:red",
    "wht_pipeline": "tab:blue",
    "stft_pipeline": "tab:orange",
}

ORDERING_COLORS = {
    "hadamard": "tab:purple",
    "sequency": "tab:brown",
    "dyadic": "tab:gray",
}


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Build benchmark plots from benchmark_results.csv"
    )
    parser.add_argument(
        "--csv",
        type=Path,
        default=Path("data/reports/summary/benchmark_results.csv"),
        help="Path to benchmark_results.csv",
    )
    parser.add_argument(
        "--out-dir",
        type=Path,
        default=Path("data/plots/benchmarks"),
        help="Output directory for PNG plots",
    )
    return parser.parse_args()


def ensure_columns(df: pd.DataFrame) -> None:
    required = {
        "method",
        "mode",
        "frame_size",
        "hop",
        "window",
        "ordering",
        "signal_len",
        "iterations",
        "total_ms",
        "avg_ms",
        "throughput_frames_per_sec",
        "throughput_samples_per_sec",
    }
    missing = sorted(required - set(df.columns))
    if missing:
        raise ValueError(f"CSV is missing columns: {missing}")


def save_pipeline_vs_length(df: pd.DataFrame, out_dir: Path) -> None:
    methods = ["wht_pipeline", "stft_pipeline"]
    sub = df[df["method"].isin(methods)].copy()
    if sub.empty:
        return

    for frame in sorted(sub["frame_size"].unique()):
        frame_df = sub[sub["frame_size"] == frame].copy()
        if frame_df.empty:
            continue

        med = (
            frame_df.groupby(["signal_len", "method"], as_index=False)["avg_ms"]
            .median()
            .sort_values(["method", "signal_len"])
        )

        plt.figure(figsize=(8, 5))
        for method in methods:
            s = med[med["method"] == method].sort_values("signal_len")
            if s.empty:
                continue
            plt.plot(
                s["signal_len"],
                s["avg_ms"],
                marker="o",
                label=method,
                color=METHOD_COLORS.get(method),
            )

        plt.xlabel("Длина сигнала, отсчёты")
        plt.ylabel("Медианное время, мс")
        plt.title(f"Зависимость времени пайплайна от длины сигнала (frame={frame})")
        plt.legend()
        plt.tight_layout()
        plt.savefig(out_dir / f"pipeline_vs_length_frame_{frame}.png", dpi=200)
        plt.close()


def save_single_frame_comparison(df: pd.DataFrame, out_dir: Path) -> None:
    methods = ["fwht", "fwht_ffht", "fft_fftw"]
    sub = df[df["method"].isin(methods)].copy()
    if sub.empty:
        return

    med = (
        sub.groupby(["frame_size", "method"], as_index=False)["avg_ms"]
        .median()
        .sort_values(["method", "frame_size"])
    )

    plt.figure(figsize=(8, 5))
    for method in methods:
        s = med[med["method"] == method].sort_values("frame_size")
        if s.empty:
            continue
        plt.plot(
            s["frame_size"],
            s["avg_ms"],
            marker="o",
            label=method,
            color=METHOD_COLORS.get(method),
        )

    plt.xlabel("Размер кадра")
    plt.ylabel("Медианное время, мс")
    plt.title("Сравнение single-frame преобразований")
    plt.legend()
    plt.tight_layout()
    plt.savefig(out_dir / "single_frame_comparison_vs_frame_size.png", dpi=200)
    plt.close()


def save_single_frame_throughput(df: pd.DataFrame, out_dir: Path) -> None:
    methods = ["fwht", "fwht_ffht", "fft_fftw"]
    sub = df[df["method"].isin(methods)].copy()
    if sub.empty:
        return

    med = (
        sub.groupby(["frame_size", "method"], as_index=False)["throughput_samples_per_sec"]
        .median()
        .sort_values(["method", "frame_size"])
    )

    plt.figure(figsize=(8, 5))
    for method in methods:
        s = med[med["method"] == method].sort_values("frame_size")
        if s.empty:
            continue
        plt.plot(
            s["frame_size"],
            s["throughput_samples_per_sec"],
            marker="o",
            label=method,
            color=METHOD_COLORS.get(method),
        )

    plt.xlabel("Размер кадра")
    plt.ylabel("Пропускная способность, отсчётов/с")
    plt.title("Пропускная способность single-frame преобразований")
    plt.legend()
    plt.tight_layout()
    plt.savefig(out_dir / "single_frame_throughput_vs_frame_size.png", dpi=200)
    plt.close()


def save_stft_wht_ratio(df: pd.DataFrame, out_dir: Path) -> None:
    sub = df[df["method"].isin(["wht_pipeline", "stft_pipeline"])].copy()
    if sub.empty:
        return

    med = (
        sub.groupby(["frame_size", "method"], as_index=False)["avg_ms"]
        .median()
        .sort_values(["frame_size", "method"])
    )

    pivot = med.pivot(index="frame_size", columns="method", values="avg_ms").reset_index()
    if "wht_pipeline" not in pivot or "stft_pipeline" not in pivot:
        return

    pivot["ratio"] = pivot["stft_pipeline"] / pivot["wht_pipeline"]

    plt.figure(figsize=(8, 5))
    plt.plot(
        pivot["frame_size"],
        pivot["ratio"],
        marker="o",
        color="tab:red",
    )
    plt.xlabel("Размер кадра")
    plt.ylabel("Отношение времени STFT / WHT")
    plt.title("Во сколько раз STFT-пайплайн медленнее WHT-пайплайна")
    plt.tight_layout()
    plt.savefig(out_dir / "stft_over_wht_ratio_vs_frame_size.png", dpi=200)
    plt.close()


def save_wht_ordering(df: pd.DataFrame, out_dir: Path) -> None:
    sub = df[df["method"] == "wht_pipeline"].copy()
    if sub.empty:
        return

    med = (
        sub.groupby(["frame_size", "ordering"], as_index=False)["avg_ms"]
        .median()
        .sort_values(["ordering", "frame_size"])
    )

    plt.figure(figsize=(8, 5))
    for ordering in sorted(x for x in med["ordering"].unique() if x != "na"):
        s = med[med["ordering"] == ordering].sort_values("frame_size")
        if s.empty:
            continue
        plt.plot(
            s["frame_size"],
            s["avg_ms"],
            marker="o",
            label=ordering,
            color=ORDERING_COLORS.get(ordering),
        )

    plt.xlabel("Размер кадра")
    plt.ylabel("Медианное время, мс")
    plt.title("Влияние порядка коэффициентов на время WHT-пайплайна")
    plt.legend()
    plt.tight_layout()
    plt.savefig(out_dir / "wht_ordering_vs_frame_size.png", dpi=200)
    plt.close()


def save_throughput(df: pd.DataFrame, out_dir: Path) -> None:
    methods = ["wht_pipeline", "stft_pipeline"]
    sub = df[df["method"].isin(methods)].copy()
    if sub.empty:
        return

    med = (
        sub.groupby(["frame_size", "method"], as_index=False)["throughput_samples_per_sec"]
        .median()
        .sort_values(["method", "frame_size"])
    )

    plt.figure(figsize=(8, 5))
    for method in methods:
        s = med[med["method"] == method].sort_values("frame_size")
        if s.empty:
            continue
        plt.plot(
            s["frame_size"],
            s["throughput_samples_per_sec"],
            marker="o",
            label=method,
            color=METHOD_COLORS.get(method),
        )

    plt.xlabel("Размер кадра")
    plt.ylabel("Пропускная способность, отсчётов/с")
    plt.title("Пропускная способность пайплайнов")
    plt.legend()
    plt.tight_layout()
    plt.savefig(out_dir / "pipeline_throughput_vs_frame_size.png", dpi=200)
    plt.close()


def main() -> None:
    args = parse_args()

    if not args.csv.is_file():
        raise FileNotFoundError(f"CSV file not found: {args.csv}")

    args.out_dir.mkdir(parents=True, exist_ok=True)

    df = pd.read_csv(args.csv)
    ensure_columns(df)

    print("Methods found in CSV:", sorted(df["method"].unique()))

    save_pipeline_vs_length(df, args.out_dir)
    save_single_frame_comparison(df, args.out_dir)
    save_single_frame_throughput(df, args.out_dir)
    save_stft_wht_ratio(df, args.out_dir)
    save_wht_ordering(df, args.out_dir)
    save_throughput(df, args.out_dir)

    print(f"Plots saved to: {args.out_dir}")
    print("Generated files:")
    for p in sorted(args.out_dir.glob("*.png")):
        print(" -", p.name)


if __name__ == "__main__":
    main()