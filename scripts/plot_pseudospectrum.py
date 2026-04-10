import argparse
import csv
from pathlib import Path

import matplotlib
matplotlib.use("Agg")

import matplotlib.pyplot as plt
import numpy as np


def read_vector_csv(path: Path) -> np.ndarray:
    with open(path, "r", encoding="utf-8") as f:
        values = [float(line.strip()) for line in f if line.strip()]
    return np.array(values, dtype=np.float32)


def read_matrix_csv(path: Path) -> np.ndarray:
    rows = []
    with open(path, "r", encoding="utf-8") as f:
        reader = csv.reader(f)
        for row in reader:
            if row:
                rows.append([float(x) for x in row])
    return np.array(rows, dtype=np.float32)


def make_extent(t: np.ndarray, f: np.ndarray):
    t0 = float(t[0]) if len(t) > 0 else 0.0
    t1 = float(t[-1]) if len(t) > 1 else t0 + 1e-3
    f0 = float(f[0]) if len(f) > 0 else 0.0
    f1 = float(f[-1]) if len(f) > 1 else f0 + 1.0
    return [t0, t1, f0, f1]


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--prefix", required=True, help="Input prefix, e.g. data/out/speech_file")
    parser.add_argument("--plots-dir", default="data/plots", help="Directory for PNG output")
    args = parser.parse_args()

    prefix = Path(args.prefix)
    plots_dir = Path(args.plots_dir)
    plots_dir.mkdir(parents=True, exist_ok=True)

    stem = prefix.name

    wht_db_path = Path(f"{prefix}_wht_db.csv")
    time_path = Path(f"{prefix}_time_s.csv")
    pseudofreq_path = Path(f"{prefix}_pseudofreq_hz.csv")

    if not wht_db_path.exists():
        raise FileNotFoundError(f"Missing file: {wht_db_path}")
    if not time_path.exists():
        raise FileNotFoundError(f"Missing file: {time_path}")
    if not pseudofreq_path.exists():
        raise FileNotFoundError(f"Missing file: {pseudofreq_path}")

    wht_db = read_matrix_csv(wht_db_path)
    t = read_vector_csv(time_path)
    f_pseudo = read_vector_csv(pseudofreq_path)

    wht_png = plots_dir / f"{stem}_wht.png"

    plt.figure(figsize=(10, 4))
    plt.imshow(
        wht_db.T,
        origin="lower",
        aspect="auto",
        extent=make_extent(t, f_pseudo),
    )
    plt.xlabel("Time, s")
    plt.ylabel("Pseudo-frequency, Hz")
    plt.title("WHT Pseudospectrogram (dB)")
    plt.colorbar(label="dB")
    plt.tight_layout()
    plt.savefig(wht_png, dpi=200)
    plt.close()

    stft_db_path = Path(f"{prefix}_stft_db.csv")
    freq_path = Path(f"{prefix}_freq_hz.csv")
    stft_time_path = Path(f"{prefix}_stft_time_s.csv")

    if stft_db_path.exists() and freq_path.exists() and stft_time_path.exists():
        stft_db = read_matrix_csv(stft_db_path)
        f = read_vector_csv(freq_path)
        t_stft = read_vector_csv(stft_time_path)

        stft_png = plots_dir / f"{stem}_stft.png"

        plt.figure(figsize=(10, 4))
        plt.imshow(
            stft_db.T,
            origin="lower",
            aspect="auto",
            extent=make_extent(t_stft, f),
        )
        plt.xlabel("Time, s")
        plt.ylabel("Frequency, Hz")
        plt.title("STFT Spectrogram (dB)")
        plt.colorbar(label="dB")
        plt.tight_layout()
        plt.savefig(stft_png, dpi=200)
        plt.close()

        print(f"Saved: {wht_png}")
        print(f"Saved: {stft_png}")
    else:
        print(f"Saved: {wht_png}")
        print("STFT CSV files not found, skipped STFT plot.")


if __name__ == "__main__":
    main()