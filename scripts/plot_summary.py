#!/usr/bin/env python3
import argparse
from pathlib import Path

import matplotlib
matplotlib.use("Agg")

import matplotlib.pyplot as plt
import pandas as pd


def plot_metric_by_window(df: pd.DataFrame, metric: str, ylabel: str, out_dir: Path) -> None:
    windows = sorted(df["window"].dropna().unique())

    for window_name in windows:
        sub = df[df["window"] == window_name].copy()
        if sub.empty:
            continue

        plt.figure(figsize=(8, 5))

        for method in sorted(sub["method"].dropna().unique()):
            s = sub[sub["method"] == method].sort_values("frame")
            if s.empty:
                continue
            plt.plot(
                s["frame"],
                s[metric],
                marker="o",
                label=method,
            )

        plt.xlabel("Размер кадра")
        plt.ylabel(ylabel)
        plt.title(f"{ylabel} по размеру кадра ({window_name})")
        plt.grid(True)
        plt.legend()
        plt.tight_layout()
        plt.savefig(out_dir / f"{metric}_{window_name}.png", dpi=200)
        plt.close()


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--input",
        default="data/reports/summary/window_sweep_summary.csv",
        help="Path to aggregated summary CSV",
    )
    parser.add_argument(
        "--plots-dir",
        default="data/plots/sweeps/summary",
        help="Directory to save plots",
    )
    args = parser.parse_args()

    input_path = Path(args.input)
    plots_dir = Path(args.plots_dir)

    if not input_path.is_file():
        raise FileNotFoundError(f"Input CSV not found: {input_path}")

    plots_dir.mkdir(parents=True, exist_ok=True)

    df = pd.read_csv(input_path)

    required = {"method", "window", "frame"}
    missing = required - set(df.columns)
    if missing:
        raise ValueError(f"Missing required columns: {sorted(missing)}")

    metric_specs = [
        ("snr_db_mean", "SNR, dB"),
        ("rmse_mean", "RMSE"),
        ("mse_mean", "MSE"),
        ("transform_ms_mean", "Время преобразования, мс"),
        ("recon_ms_mean", "Время реконструкции, мс"),
        ("total_ms_mean", "Общее время, мс"),
    ]

    for metric, ylabel in metric_specs:
        if metric in df.columns:
            plot_metric_by_window(df, metric, ylabel, plots_dir)

    print(f"Saved plots to: {plots_dir}")
    for p in sorted(plots_dir.glob("*.png")):
        print(" -", p.name)


if __name__ == "__main__":
    main()