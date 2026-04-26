#!/usr/bin/env python3
from __future__ import annotations

import argparse
import csv
import random
from pathlib import Path

import numpy as np
import soundfile as sf


AUDIO_EXTS = {".wav", ".flac"}


def find_audio_files(root: Path) -> list[Path]:
    return sorted(
        p for p in root.rglob("*")
        if p.is_file() and p.suffix.lower() in AUDIO_EXTS
    )


def to_mono(x: np.ndarray) -> np.ndarray:
    if x.ndim == 1:
        return x.astype(np.float32)
    return np.mean(x, axis=1).astype(np.float32)


def rms(x: np.ndarray) -> float:
    return float(np.sqrt(np.mean(np.square(x), dtype=np.float64)))


def add_awgn_for_snr_db(x: np.ndarray, snr_db: float, rng: np.random.Generator) -> np.ndarray:
    signal_rms = max(rms(x), 1e-12)
    noise_rms = signal_rms / (10.0 ** (snr_db / 20.0))
    noise = rng.normal(0.0, noise_rms, size=x.shape).astype(np.float32)
    y = x + noise

    peak = float(np.max(np.abs(y))) if y.size else 1.0
    if peak > 0.999:
        y = y / peak * 0.999

    return y.astype(np.float32)


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Pick one random Librispeech file and create noisy copies at several SNR levels."
    )
    parser.add_argument(
        "--input-root",
        default="data/raw/librispeech_subset",
        help="Root directory of librispeech_subset",
    )
    parser.add_argument(
        "--out-dir",
        default="data/raw/librispeech_subset_noisy",
        help="Where to save noisy files",
    )
    parser.add_argument(
        "--snrs",
        type=float,
        nargs="+",
        default=[30, 20, 10, 5],
        help="Target SNR values in dB",
    )
    parser.add_argument(
        "--seed",
        type=int,
        default=42,
        help="Random seed",
    )
    parser.add_argument(
        "--copy-clean",
        action="store_true",
        help="Also copy the selected clean file into out-dir/clean",
    )
    args = parser.parse_args()

    rng = np.random.default_rng(args.seed)
    py_rng = random.Random(args.seed)

    input_root = Path(args.input_root)
    out_dir = Path(args.out_dir)
    out_dir.mkdir(parents=True, exist_ok=True)

    files = find_audio_files(input_root)
    if not files:
        raise FileNotFoundError(f"No audio files found in {input_root}")

    chosen = py_rng.choice(files)
    x, sr = sf.read(chosen, always_2d=False)
    x = to_mono(np.asarray(x))

    stem = chosen.stem
    rel = chosen.relative_to(input_root)

    rows: list[dict[str, str | float | int]] = []

    if args.copy_clean:
        clean_dir = out_dir / "clean"
        clean_dir.mkdir(parents=True, exist_ok=True)
        clean_path = clean_dir / f"{stem}.wav"
        sf.write(clean_path, x, sr)
        rows.append({
            "kind": "clean",
            "snr_db": "",
            "source_file": str(rel),
            "output_file": str(clean_path.relative_to(out_dir)),
            "sample_rate": sr,
        })

    for snr_db in args.snrs:
        y = add_awgn_for_snr_db(x, snr_db, rng)
        snr_tag = f"snr{int(snr_db)}"
        target_dir = out_dir / snr_tag
        target_dir.mkdir(parents=True, exist_ok=True)

        out_path = target_dir / f"{stem}_{snr_tag}.wav"
        sf.write(out_path, y, sr)

        rows.append({
            "kind": "noisy",
            "snr_db": snr_db,
            "source_file": str(rel),
            "output_file": str(out_path.relative_to(out_dir)),
            "sample_rate": sr,
        })

    meta_path = out_dir / "metadata.csv"
    with open(meta_path, "w", newline="", encoding="utf-8") as f:
        writer = csv.DictWriter(
            f,
            fieldnames=["kind", "snr_db", "source_file", "output_file", "sample_rate"],
        )
        writer.writeheader()
        writer.writerows(rows)

    print(f"Selected source: {chosen}")
    print(f"Saved outputs to: {out_dir}")
    print(f"Metadata: {meta_path}")


if __name__ == "__main__":
    main()