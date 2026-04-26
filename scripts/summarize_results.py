#!/usr/bin/env python3
import argparse
import math
from pathlib import Path

import pandas as pd


NUMERIC_FIELDS = [
    "mse",
    "rmse",
    "max_abs",
    "snr_db",
    "transform_ms",
    "recon_ms",
    "total_ms",
]


def clean_numeric(df: pd.DataFrame, columns: list[str]) -> pd.DataFrame:
    out = df.copy()
    for col in columns:
        if col in out.columns:
            out[col] = pd.to_numeric(out[col], errors="coerce")
            out.loc[~out[col].map(lambda x: pd.isna(x) or math.isfinite(x)), col] = pd.NA
    return out


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--input",
        default="data/reports/summary/window_sweep_metrics.csv",
        help="Input detailed metrics CSV",
    )
    parser.add_argument(
        "--output",
        default="data/reports/summary/window_sweep_summary.csv",
        help="Output aggregated summary CSV",
    )
    args = parser.parse_args()

    input_path = Path(args.input)
    output_path = Path(args.output)

    if not input_path.is_file():
        raise FileNotFoundError(f"Input CSV not found: {input_path}")

    df = pd.read_csv(input_path)

    required_group_cols = ["method", "window", "frame", "hop", "ordering"]
    missing_group = [c for c in required_group_cols if c not in df.columns]
    if missing_group:
        raise ValueError(f"Missing required grouping columns: {missing_group}")

    df = clean_numeric(df, NUMERIC_FIELDS)

    agg_spec = {"n": ("method", "size")}
    for field in NUMERIC_FIELDS:
        if field in df.columns:
            agg_spec[f"{field}_mean"] = (field, "mean")

    summary = (
        df.groupby(required_group_cols, dropna=False, as_index=False)
          .agg(**agg_spec)
          .sort_values(required_group_cols)
    )

    output_path.parent.mkdir(parents=True, exist_ok=True)
    summary.to_csv(output_path, index=False)

    print(f"Saved: {output_path}")
    print(f"Rows: {len(summary)}")
    print("Columns:", list(summary.columns))


if __name__ == "__main__":
    main()