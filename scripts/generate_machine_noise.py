#!/usr/bin/env python3

import argparse
import math
import wave
from pathlib import Path

import numpy as np


def normalize_to_int16(x: np.ndarray) -> np.ndarray:
    peak = float(np.max(np.abs(x))) if x.size else 1.0
    peak = max(peak, 1e-12)
    y = x / peak
    return np.clip(y * 32767.0, -32768, 32767).astype(np.int16)


def write_wav(path: Path, fs: int, x: np.ndarray) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    pcm = normalize_to_int16(x)

    with wave.open(str(path), "wb") as wf:
        wf.setnchannels(1)
        wf.setsampwidth(2)  # int16
        wf.setframerate(fs)
        wf.writeframes(pcm.tobytes())


def make_machine_hum(fs: int, dur: float, base_freq: float = 50.0, noise_level: float = 0.05) -> np.ndarray:
    t = np.arange(int(fs * dur), dtype=np.float32) / fs

    x = (
        0.55 * np.sin(2.0 * math.pi * base_freq * t) +
        0.22 * np.sin(2.0 * math.pi * 2.0 * base_freq * t + 0.3) +
        0.12 * np.sin(2.0 * math.pi * 3.0 * base_freq * t + 1.1)
    )

    env = 0.75 + 0.25 * np.sin(2.0 * math.pi * 0.7 * t)
    x *= env
    x += noise_level * np.random.randn(len(t)).astype(np.float32)
    return x.astype(np.float32)


def make_rotating_machine(fs: int, dur: float, rpm: float = 1500.0, noise_level: float = 0.08) -> np.ndarray:
    t = np.arange(int(fs * dur), dtype=np.float32) / fs
    shaft_freq = rpm / 60.0

    x = (
        0.45 * np.sin(2.0 * math.pi * shaft_freq * t) +
        0.20 * np.sin(2.0 * math.pi * 2.0 * shaft_freq * t) +
        0.10 * np.sin(2.0 * math.pi * 5.0 * shaft_freq * t)
    )

    env = 0.7 + 0.3 * np.maximum(0.0, np.sin(2.0 * math.pi * 1.5 * t))
    x *= env
    x += noise_level * np.random.randn(len(t)).astype(np.float32)
    return x.astype(np.float32)


def make_faulty_machine(
    fs: int,
    dur: float,
    base_freq: float = 47.0,
    click_rate: float = 6.0,
    noise_level: float = 0.05,
) -> np.ndarray:
    t = np.arange(int(fs * dur), dtype=np.float32) / fs

    x = (
        0.40 * np.sin(2.0 * math.pi * base_freq * t) +
        0.18 * np.sin(2.0 * math.pi * 2.0 * base_freq * t + 0.5)
    )
    x += noise_level * np.random.randn(len(t)).astype(np.float32)

    click_positions = np.arange(0.0, dur, 1.0 / click_rate)
    for pos in click_positions:
        idx = int(pos * fs)
        width = max(8, int(0.003 * fs))
        if idx + width < len(x):
            pulse = np.hanning(width).astype(np.float32) * 0.7
            x[idx:idx + width] += pulse

    return x.astype(np.float32)


def make_impulse_train(
    fs: int,
    dur: float,
    click_rate: float = 8.0,
    noise_level: float = 0.02,
) -> np.ndarray:
    x = np.zeros(int(fs * dur), dtype=np.float32)
    click_positions = np.arange(0.0, dur, 1.0 / click_rate)

    for pos in click_positions:
        idx = int(pos * fs)
        width = max(8, int(0.002 * fs))
        if idx + width < len(x):
            pulse = np.hanning(width).astype(np.float32)
            x[idx:idx + width] += pulse

    x += noise_level * np.random.randn(len(x)).astype(np.float32)
    return x.astype(np.float32)


def main() -> None:
    parser = argparse.ArgumentParser(description="Generate synthetic machine-like WAV files.")
    parser.add_argument("--out-dir", default="data/wav", help="Output directory")
    parser.add_argument("--fs", type=int, default=16000, help="Sample rate")
    parser.add_argument("--dur", type=float, default=6.0, help="Duration in seconds")
    parser.add_argument("--seed", type=int, default=42, help="Random seed")
    args = parser.parse_args()

    np.random.seed(args.seed)
    out_dir = Path(args.out_dir)
    out_dir.mkdir(parents=True, exist_ok=True)

    hum = make_machine_hum(fs=args.fs, dur=args.dur, base_freq=50.0)
    rotating = make_rotating_machine(fs=args.fs, dur=args.dur, rpm=1500.0)
    faulty = make_faulty_machine(fs=args.fs, dur=args.dur, base_freq=47.0, click_rate=6.0)
    impulse = make_impulse_train(fs=args.fs, dur=args.dur, click_rate=8.0)

    write_wav(out_dir / "machine_hum.wav", args.fs, hum)
    write_wav(out_dir / "rotating_machine.wav", args.fs, rotating)
    write_wav(out_dir / "faulty_machine.wav", args.fs, faulty)
    write_wav(out_dir / "impulse_train.wav", args.fs, impulse)

    print(f"Saved WAV files to: {out_dir}")
    print("  - machine_hum.wav")
    print("  - rotating_machine.wav")
    print("  - faulty_machine.wav")
    print("  - impulse_train.wav")


if __name__ == "__main__":
    main()
