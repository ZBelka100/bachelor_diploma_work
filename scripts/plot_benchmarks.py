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
        description="Build benchmark plots from benchmark_results CSV"
    )
    parser.add_argument(
        "--csv",
        type=Path,
        required=True,
        help="Path to benchmark results CSV",
    )
    parser.add_argument(
        "--out-dir",
        type=Path,
        required=True,
        help="Output directory for PNG plots",
    )
    return parser.parse_args()


def ensure_columns(df: pd.DataFrame) -> None:
    required = {
        "method",
        "mode",
        "fft_impl",
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


def grouped_median(df: pd.DataFrame, keys: list[str], value: str) -> pd.DataFrame:
    return (
        df.groupby(keys, as_index=False)[value]
        .median()
        .sort_values(keys)
    )


def save_single_frame_comparison(df: pd.DataFrame, out_dir: Path) -> None:
    sub = df[df["mode"] == "single_frame"].copy()
    methods = ["fwht", "fwht_ffht", "fft_fftw"]
    sub = sub[sub["method"].isin(methods)]
    if sub.empty:
        return

    med = grouped_median(sub, ["frame_size", "method"], "avg_ms")

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
    sub = df[df["mode"] == "single_frame"].copy()
    methods = ["fwht", "fwht_ffht", "fft_fftw"]
    sub = sub[sub["method"].isin(methods)]
    if sub.empty:
        return

    med = grouped_median(sub, ["frame_size", "method"], "throughput_samples_per_sec")

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


def save_pipeline_vs_length(df: pd.DataFrame, out_dir: Path, mode: str) -> None:
    methods = ["wht_pipeline", "stft_pipeline"]
    sub = df[(df["method"].isin(methods)) & (df["mode"] == mode)].copy()
    if sub.empty:
        return

    for frame in sorted(sub["frame_size"].unique()):
        frame_df = sub[sub["frame_size"] == frame].copy()
        if frame_df.empty:
            continue

        med = grouped_median(frame_df, ["signal_len", "method"], "avg_ms")

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
        plt.title(f"Зависимость времени пайплайна от длины сигнала ({mode}, frame={frame})")
        plt.legend()
        plt.tight_layout()
        plt.savefig(out_dir / f"pipeline_{mode}_vs_length_frame_{frame}.png", dpi=200)
        plt.close()


def save_pipeline_vs_frame(df: pd.DataFrame, out_dir: Path, mode: str) -> None:
    methods = ["wht_pipeline", "stft_pipeline"]
    sub = df[(df["method"].isin(methods)) & (df["mode"] == mode)].copy()
    if sub.empty:
        return

    med = grouped_median(sub, ["frame_size", "method"], "avg_ms")

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
    plt.title(f"Сравнение пайплайнов по frame size ({mode})")
    plt.legend()
    plt.tight_layout()
    plt.savefig(out_dir / f"pipeline_{mode}_vs_frame_size.png", dpi=200)
    plt.close()


def save_pipeline_throughput(df: pd.DataFrame, out_dir: Path, mode: str) -> None:
    methods = ["wht_pipeline", "stft_pipeline"]
    sub = df[(df["method"].isin(methods)) & (df["mode"] == mode)].copy()
    if sub.empty:
        return

    med = grouped_median(sub, ["frame_size", "method"], "throughput_samples_per_sec")

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
    plt.title(f"Пропускная способность пайплайнов ({mode})")
    plt.legend()
    plt.tight_layout()
    plt.savefig(out_dir / f"pipeline_{mode}_throughput_vs_frame_size.png", dpi=200)
    plt.close()


def save_stft_wht_ratio(df: pd.DataFrame, out_dir: Path, mode: str) -> None:
    sub = df[
        (df["method"].isin(["wht_pipeline", "stft_pipeline"])) &
        (df["mode"] == mode)
    ].copy()
    if sub.empty:
        return

    med = grouped_median(sub, ["frame_size", "method"], "avg_ms")
    pivot = med.pivot(index="frame_size", columns="method", values="avg_ms").reset_index()

    if "wht_pipeline" not in pivot or "stft_pipeline" not in pivot:
        return

    pivot["ratio"] = pivot["stft_pipeline"] / pivot["wht_pipeline"]

    plt.figure(figsize=(8, 5))
    plt.plot(pivot["frame_size"], pivot["ratio"], marker="o", color="tab:red")
    plt.xlabel("Размер кадра")
    plt.ylabel("Отношение времени STFT / WHT")
    plt.title(f"Во сколько раз STFT медленнее WHT ({mode})")
    plt.tight_layout()
    plt.savefig(out_dir / f"stft_over_wht_ratio_{mode}_vs_frame_size.png", dpi=200)
    plt.close()


def save_wht_ordering(df: pd.DataFrame, out_dir: Path, mode: str) -> None:
    sub = df[(df["method"] == "wht_pipeline") & (df["mode"] == mode)].copy()
    if sub.empty:
        return

    med = grouped_median(sub, ["frame_size", "ordering"], "avg_ms")

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
    plt.title(f"Влияние порядка коэффициентов на WHT ({mode})")
    plt.legend()
    plt.tight_layout()
    plt.savefig(out_dir / f"wht_ordering_{mode}_vs_frame_size.png", dpi=200)
    plt.close()


def save_stft_backend_comparison(df: pd.DataFrame, out_dir: Path, mode: str) -> None:
    sub = df[(df["method"] == "stft_pipeline") & (df["mode"] == mode)].copy()
    if sub.empty or "fft_impl" not in sub.columns:
        return

    med = grouped_median(sub, ["frame_size", "fft_impl"], "avg_ms")

    plt.figure(figsize=(8, 5))
    for backend in sorted(x for x in med["fft_impl"].unique() if x != "na"):
        s = med[med["fft_impl"] == backend].sort_values("frame_size")
        if s.empty:
            continue
        plt.plot(s["frame_size"], s["avg_ms"], marker="o", label=backend)

    plt.xlabel("Размер кадра")
    plt.ylabel("Медианное время, мс")
    plt.title(f"Сравнение backend для STFT ({mode})")
    plt.legend()
    plt.tight_layout()
    plt.savefig(out_dir / f"stft_backend_{mode}_vs_frame_size.png", dpi=200)
    plt.close()


def main() -> None:
    args = parse_args()

    if not args.csv.is_file():
        raise FileNotFoundError(f"CSV file not found: {args.csv}")

    args.out_dir.mkdir(parents=True, exist_ok=True)

    df = pd.read_csv(args.csv)
    ensure_columns(df)

    print("Methods found in CSV:", sorted(df["method"].unique()))
    print("Modes found in CSV:", sorted(df["mode"].unique()))

    save_single_frame_comparison(df, args.out_dir)
    save_single_frame_throughput(df, args.out_dir)

    for mode in ["transform_only", "full_cycle"]:
        save_pipeline_vs_length(df, args.out_dir, mode)
        save_pipeline_vs_frame(df, args.out_dir, mode)
        save_pipeline_throughput(df, args.out_dir, mode)
        save_stft_wht_ratio(df, args.out_dir, mode)
        save_wht_ordering(df, args.out_dir, mode)
        save_stft_backend_comparison(df, args.out_dir, mode)

    print(f"Plots saved to: {args.out_dir}")
    print("Generated files:")
    for p in sorted(args.out_dir.glob("*.png")):
        print(" -", p.name)


if __name__ == "__main__":
    main()