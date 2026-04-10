import argparse
from pathlib import Path

import matplotlib
matplotlib.use("Agg")

import matplotlib.pyplot as plt
import pandas as pd


def make_bar(df, metric, out_path):
    pivot = df.pivot_table(
        index="group",
        columns=["transform", "order"],
        values=metric,
        aggfunc="mean"
    )
    ax = pivot.plot(kind="bar", figsize=(10, 5))
    ax.set_ylabel(metric)
    ax.set_title(metric)
    plt.tight_layout()
    plt.savefig(out_path, dpi=200)
    plt.close()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--input", default="results/results_summary.csv")
    parser.add_argument("--plots-dir", default="results/plots")
    args = parser.parse_args()

    plots_dir = Path(args.plots_dir)
    plots_dir.mkdir(parents=True, exist_ok=True)

    df = pd.read_csv(args.input)

    metrics = [
        "snr_db_mean",
        "rmse_mean",
        "energy_topk_mean",
        "peak_ratio_mean",
        "sparsity_ratio_db60_mean",
    ]

    for metric in metrics:
        make_bar(df, metric, plots_dir / f"{metric}.png")

    print(f"Saved plots to: {plots_dir}")


if __name__ == "__main__":
    main()