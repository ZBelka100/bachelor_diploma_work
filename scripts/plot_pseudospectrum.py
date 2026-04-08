import argparse
import csv
from pathlib import Path

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


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--prefix", required=True, help="Prefix like out")
    args = parser.parse_args()

    prefix = Path(args.prefix)

    wht_db = read_matrix_csv(Path(f"{prefix}_wht_db.csv"))
    t = read_vector_csv(Path(f"{prefix}_time_s.csv"))
    f_pseudo = read_vector_csv(Path(f"{prefix}_pseudofreq_hz.csv"))

    plt.figure(figsize=(10, 4))
    plt.imshow(
        wht_db.T,
        origin="lower",
        aspect="auto",
        extent=[t[0], t[-1] if len(t) > 1 else t[0] + 1e-3, f_pseudo[0], f_pseudo[-1]],
    )
    plt.xlabel("Time, s")
    plt.ylabel("Pseudo-frequency, Hz")
    plt.title("STWHT Pseudospectrogram (dB)")
    plt.colorbar(label="dB")
    plt.tight_layout()
    plt.savefig(f"{prefix}_wht.png", dpi=200)

    stft_db_path = Path(f"{prefix}_stft_db.csv")
    freq_path = Path(f"{prefix}_freq_hz.csv")
    stft_time_path = Path(f"{prefix}_stft_time_s.csv")

    if stft_db_path.exists() and freq_path.exists() and stft_time_path.exists():
        stft_db = read_matrix_csv(stft_db_path)
        f = read_vector_csv(freq_path)
        t_stft = read_vector_csv(stft_time_path)

        plt.figure(figsize=(10, 4))
        plt.imshow(
            stft_db.T,
            origin="lower",
            aspect="auto",
            extent=[t_stft[0], t_stft[-1] if len(t_stft) > 1 else t_stft[0] + 1e-3, f[0], f[-1]],
        )
        plt.xlabel("Time, s")
        plt.ylabel("Frequency, Hz")
        plt.title("STFT Spectrogram (dB)")
        plt.colorbar(label="dB")
        plt.tight_layout()
        plt.savefig(f"{prefix}_stft.png", dpi=200)

    print("Saved plots.")


if __name__ == "__main__":
    main()