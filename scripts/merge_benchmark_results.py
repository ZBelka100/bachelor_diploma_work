#!/usr/bin/env python3
from pathlib import Path
import argparse
import pandas as pd


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--custom", required=True)
    parser.add_argument("--fftw", required=True)
    parser.add_argument("--out", required=True)
    args = parser.parse_args()

    df_custom = pd.read_csv(args.custom)
    df_fftw = pd.read_csv(args.fftw)

    if "fft_impl" not in df_custom.columns:
        df_custom["fft_impl"] = "custom"
    if "fft_impl" not in df_fftw.columns:
        df_fftw["fft_impl"] = "fftw"

    df = pd.concat([df_custom, df_fftw], ignore_index=True)
    Path(args.out).parent.mkdir(parents=True, exist_ok=True)
    df.to_csv(args.out, index=False)

    print("Saved:", args.out)
    print("Rows:", len(df))
    print("Methods:", sorted(df["method"].unique()))
    print("Modes:", sorted(df["mode"].unique()))
    print("FFT impl:", sorted(df["fft_impl"].dropna().unique()))


if __name__ == "__main__":
    main()