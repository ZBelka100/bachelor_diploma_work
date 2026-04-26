#!/usr/bin/env python3
from __future__ import annotations

import argparse
from pathlib import Path

import matplotlib
matplotlib.use("Agg")

import matplotlib.pyplot as plt
import pandas as pd


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    parser.add_argument("--csv", required=True, type=Path)
    parser.add_argument("--out-dir", required=True, type=Path)
    return parser.parse_args()


def grouped_median(df: pd.DataFrame, keys: list[str], value: str) -> pd.DataFrame:
    return (
        df.groupby(keys, as_index=False)[value]
        .median()
        .sort_values(keys)
    )


def save_pipeline_compare(df: pd.DataFrame, out_dir: Path, mode: str, metric: str, ylabel: str, filename: str) -> None:
    sub = df[df["mode"] == mode].copy()
    if sub.empty:
        return

    # WHT берём только из custom, чтобы не дублировать одинаковые линии
    wht = sub[(sub["method"] == "wht_pipeline") & (sub["fft_impl"].isin(["custom", "na"]))].copy()
    if not wht.empty:
        wht = grouped_median(wht, ["frame_size"], metric)
        wht["series"] = "wht_pipeline"

    stft_custom = sub[(sub["method"] == "stft_pipeline") & (sub["fft_impl"] == "custom")].copy()
    if not stft_custom.empty:
        stft_custom = grouped_median(stft_custom, ["frame_size"], metric)
        stft_custom["series"] = "stft_pipeline_custom"

    stft_fftw = sub[(sub["method"] == "stft_pipeline") & (sub["fft_impl"] == "fftw")].copy()
    if not stft_fftw.empty:
        stft_fftw = grouped_median(stft_fftw, ["frame_size"], metric)
        stft_fftw["series"] = "stft_pipeline_fftw"

    plot_df = pd.concat(
        [x for x in [wht if 'wht' in locals() else None, stft_custom if 'stft_custom' in locals() else None, stft_fftw if 'stft_fftw' in locals() else None] if x is not None and not x.empty],
        ignore_index=True
    )

    if plot_df.empty:
        return

    plt.figure(figsize=(8, 5))

    order = ["wht_pipeline", "stft_pipeline_custom", "stft_pipeline_fftw"]
    for series in order:
        s = plot_df[plot_df["series"] == series].sort_values("frame_size")
        if s.empty:
            continue
        plt.plot(s["frame_size"], s[metric], marker="o", label=series)

    plt.xlabel("Размер кадра")
    plt.ylabel(ylabel)
    plt.title(f"{ylabel} ({mode})")
    plt.grid(True)
    plt.legend()
    plt.tight_layout()
    plt.savefig(out_dir / filename, dpi=200)
    plt.close()


def save_single_frame_compare(df: pd.DataFrame, out_dir: Path) -> None:
    sub = df[df["mode"] == "single_frame"].copy()
    if sub.empty:
        return

    methods = ["fwht", "fwht_ffht"]
    med = grouped_median(sub[sub["method"].isin(methods)], ["frame_size", "method"], "avg_ms")

    plt.figure(figsize=(8, 5))
    for method in methods:
        s = med[med["method"] == method].sort_values("frame_size")
        if s.empty:
            continue
        plt.plot(s["frame_size"], s["avg_ms"], marker="o", label=method)

    plt.xlabel("Размер кадра")
    plt.ylabel("Медианное время, мс")
    plt.title("Single-frame WHT benchmark")
    plt.grid(True)
    plt.legend()
    plt.tight_layout()
    plt.savefig(out_dir / "single_frame_wht_compare.png", dpi=200)
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
        plt.plot(s["frame_size"], s["avg_ms"], marker="o", label=ordering)

    plt.xlabel("Размер кадра")
    plt.ylabel("Медианное время, мс")
    plt.title(f"WHT pipeline по порядкам ({mode})")
    plt.grid(True)
    plt.legend()
    plt.tight_layout()
    plt.savefig(out_dir / f"wht_ordering_{mode}.png", dpi=200)
    plt.close()


def main() -> None:
    args = parse_args()
    args.out_dir.mkdir(parents=True, exist_ok=True)

    df = pd.read_csv(args.csv)

    save_pipeline_compare(
        df, args.out_dir,
        mode="transform_only",
        metric="avg_ms",
        ylabel="Медианное время, мс",
        filename="pipeline_transform_only_avg_ms.png",
    )
    save_pipeline_compare(
        df, args.out_dir,
        mode="full_cycle",
        metric="avg_ms",
        ylabel="Медианное время, мс",
        filename="pipeline_full_cycle_avg_ms.png",
    )
    save_pipeline_compare(
        df, args.out_dir,
        mode="transform_only",
        metric="throughput_samples_per_sec",
        ylabel="Пропускная способность, отсчётов/с",
        filename="pipeline_transform_only_throughput.png",
    )
    save_pipeline_compare(
        df, args.out_dir,
        mode="full_cycle",
        metric="throughput_samples_per_sec",
        ylabel="Пропускная способность, отсчётов/с",
        filename="pipeline_full_cycle_throughput.png",
    )

    save_single_frame_compare(df, args.out_dir)
    save_wht_ordering(df, args.out_dir, "transform_only")
    save_wht_ordering(df, args.out_dir, "full_cycle")

    print(f"Plots saved to: {args.out_dir}")
    for p in sorted(args.out_dir.glob("*.png")):
        print(" -", p.name)


if __name__ == "__main__":
    main()