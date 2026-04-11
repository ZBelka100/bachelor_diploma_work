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


def sanitize_db_limits(x: np.ndarray, vmin: float, vmax: float) -> np.ndarray:
    x = np.asarray(x, dtype=np.float32)
    x = np.nan_to_num(x, nan=vmin, posinf=vmax, neginf=vmin)
    return np.clip(x, vmin, vmax)


def save_image(
    matrix_db: np.ndarray,
    t: np.ndarray,
    f: np.ndarray,
    out_path: Path,
    title: str,
    ylabel: str,
    vmin: float,
    vmax: float,
) -> None:
    matrix_db = sanitize_db_limits(matrix_db, vmin=vmin, vmax=vmax)

    plt.figure(figsize=(10, 4))
    plt.imshow(
        matrix_db.T,
        origin="lower",
        aspect="auto",
        extent=make_extent(t, f),
        vmin=vmin,
        vmax=vmax,
    )
    plt.xlabel("Time, s")
    plt.ylabel(ylabel)
    plt.title(title)
    plt.colorbar(label="dB")
    plt.tight_layout()
    plt.savefig(out_path, dpi=200, bbox_inches="tight")
    plt.close()


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--prefix",
        required=True,
        help="Input prefix, e.g. data/out/.../speech_file",
    )
    parser.add_argument(
        "--plots-dir",
        default=None,
        help="Directory for PNG output. If omitted, PNGs are saved next to the prefix files.",
    )
    parser.add_argument(
        "--make-subdir",
        action="store_true",
        help="Create a subdirectory named after prefix stem inside plots-dir.",
    )
    parser.add_argument(
        "--vmin",
        type=float,
        default=-120.0,
        help="Lower dB limit for colormap.",
    )
    parser.add_argument(
        "--vmax",
        type=float,
        default=0.0,
        help="Upper dB limit for colormap.",
    )
    args = parser.parse_args()

    prefix = Path(args.prefix)
    stem = prefix.name

    if args.plots_dir is None:
        plots_dir = prefix.parent
    else:
        plots_dir = Path(args.plots_dir)

    if args.make_subdir:
        plots_dir = plots_dir / stem

    plots_dir.mkdir(parents=True, exist_ok=True)

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
    save_image(
        matrix_db=wht_db,
        t=t,
        f=f_pseudo,
        out_path=wht_png,
        title="WHT Pseudospectrogram (dB)",
        ylabel="Pseudo-frequency, Hz",
        vmin=args.vmin,
        vmax=args.vmax,
    )

    stft_db_path = Path(f"{prefix}_stft_db.csv")
    freq_path = Path(f"{prefix}_freq_hz.csv")
    stft_time_path = Path(f"{prefix}_stft_time_s.csv")

    print(f"Saved: {wht_png}")

    if stft_db_path.exists() and freq_path.exists() and stft_time_path.exists():
        stft_db = read_matrix_csv(stft_db_path)
        f = read_vector_csv(freq_path)
        t_stft = read_vector_csv(stft_time_path)

        stft_png = plots_dir / f"{stem}_stft.png"
        save_image(
            matrix_db=stft_db,
            t=t_stft,
            f=f,
            out_path=stft_png,
            title="STFT Spectrogram (dB)",
            ylabel="Frequency, Hz",
            vmin=args.vmin,
            vmax=args.vmax,
        )

        print(f"Saved: {stft_png}")
    else:
        print("STFT CSV files not found, skipped STFT plot.")


if __name__ == "__main__":
    main()