import argparse
import csv
import math
from pathlib import Path
from typing import Optional, Tuple, Dict, List

import numpy as np

try:
    import soundfile as sf
except ImportError:
    sf = None


ORDER_SET = {"sequency", "hadamard", "dyadic"}
AUDIO_EXTS = {".wav", ".flac"}


def read_matrix_csv(path: Path) -> np.ndarray:
    rows = []
    with open(path, "r", encoding="utf-8") as f:
        reader = csv.reader(f)
        for row in reader:
            if row:
                rows.append([float(x) for x in row])
    return np.asarray(rows, dtype=np.float64)


def compute_audio_metrics(original: np.ndarray, reconstructed: np.ndarray) -> Dict[str, float]:
    n = min(len(original), len(reconstructed))
    if n == 0:
        return {"mse": math.nan, "rmse": math.nan, "max_abs": math.nan, "snr_db": math.nan}

    x = original[:n]
    y = reconstructed[:n]
    err = x - y

    mse = float(np.mean(err ** 2))
    rmse = float(np.sqrt(mse))
    max_abs = float(np.max(np.abs(err)))

    signal_power = float(np.mean(x ** 2))
    noise_power = float(np.mean(err ** 2))
    if noise_power <= 1e-20:
        snr_db = float("inf")
    else:
        snr_db = 10.0 * math.log10(max(signal_power, 1e-20) / noise_power)

    return {
        "mse": mse,
        "rmse": rmse,
        "max_abs": max_abs,
        "snr_db": snr_db,
    }


def energy_compaction_top_k(matrix: np.ndarray, k_ratio: float = 0.05) -> float:
    flat = np.abs(matrix).ravel()
    if flat.size == 0:
        return math.nan
    energy = flat ** 2
    total = float(np.sum(energy))
    if total <= 1e-20:
        return 0.0
    k = max(1, int(len(energy) * k_ratio))
    topk = np.partition(energy, -k)[-k:]
    return float(np.sum(topk) / total)


def peak_to_total_ratio(matrix: np.ndarray) -> float:
    flat = np.abs(matrix).ravel()
    if flat.size == 0:
        return math.nan
    energy = flat ** 2
    total = float(np.sum(energy))
    if total <= 1e-20:
        return 0.0
    peak = float(np.max(energy))
    return peak / total


def sparsity_ratio(matrix: np.ndarray, db_threshold: float = -60.0) -> float:
    flat = matrix.ravel()
    if flat.size == 0:
        return math.nan
    return float(np.mean(flat > db_threshold))


def build_audio_index(raw_root: Path) -> Dict[str, List[Path]]:
    index: Dict[str, List[Path]] = {}
    for p in raw_root.rglob("*"):
        if p.is_file() and p.suffix.lower() in AUDIO_EXTS:
            index.setdefault(p.stem, []).append(p)
    return index


def parse_prefix_name(prefix_name: str) -> Tuple[str, str, str]:
    parts = prefix_name.split("_")
    if len(parts) >= 2 and parts[-1] in ORDER_SET:
        order = parts[-1]
        group = parts[0]
        file_stem = "_".join(parts[1:-1])
    else:
        order = "-"
        group = parts[0] if parts else "unknown"
        file_stem = "_".join(parts[1:]) if len(parts) > 1 else prefix_name
    return group, file_stem, order


def find_original_audio(file_stem: str, group: str, audio_index: Dict[str, List[Path]]) -> Optional[Path]:
    candidates = audio_index.get(file_stem, [])
    if not candidates:
        return None
    if len(candidates) == 1:
        return candidates[0]

    group_lower = group.lower()
    for c in candidates:
        s = str(c).lower()
        if group_lower in s:
            return c
        if group_lower.startswith("machine") and ("machine" in s or "mimii" in s):
            return c
        if group_lower == "speech" and ("speech" in s or "librispeech" in s):
            return c
        if group_lower == "events" and ("events" in s or "esc" in s):
            return c
    return candidates[0]


def read_audio(path: Path) -> Tuple[np.ndarray, int]:
    if sf is None:
        raise RuntimeError("Install soundfile: python3 -m pip install soundfile")
    data, sr = sf.read(str(path), always_2d=False)
    if getattr(data, "ndim", 1) > 1:
        data = np.mean(data, axis=1)
    return data.astype(np.float64), int(sr)


def detect_all_prefixes(out_dir: Path) -> List[Path]:
    prefixes = []
    seen = set()
    for p in out_dir.glob("*_wht_db.csv"):
        prefix = Path(str(p).replace("_wht_db.csv", ""))
        if str(prefix) not in seen:
            prefixes.append(prefix)
            seen.add(str(prefix))
    return sorted(prefixes)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--raw-root", default="data/raw")
    parser.add_argument("--out-dir", default="data/out")
    parser.add_argument("--plots-dir", default="data/plots")
    parser.add_argument("--results-dir", default="results")
    parser.add_argument("--topk-ratio", type=float, default=0.05)
    args = parser.parse_args()

    raw_root = Path(args.raw_root)
    out_dir = Path(args.out_dir)
    plots_dir = Path(args.plots_dir)
    results_dir = Path(args.results_dir)
    results_dir.mkdir(parents=True, exist_ok=True)

    audio_index = build_audio_index(raw_root)
    prefixes = detect_all_prefixes(out_dir)

    rows = []

    for prefix in prefixes:
        prefix_name = prefix.name
        group, file_stem, order = parse_prefix_name(prefix_name)

        original_path = find_original_audio(file_stem, group, audio_index)
        recon_path = Path(f"{prefix}_recon.wav")

        wht_db_path = Path(f"{prefix}_wht_db.csv")
        stft_db_path = Path(f"{prefix}_stft_db.csv")

        wht_png = plots_dir / f"{prefix_name}_wht.png"
        stft_png = plots_dir / f"{prefix_name}_stft.png"

        if not wht_db_path.exists():
            continue

        wht_db = read_matrix_csv(wht_db_path)

        row_wht = {
            "file_name": file_stem,
            "group": group,
            "transform": "wht",
            "order": order,
            "original_audio": str(original_path) if original_path else "",
            "reconstructed_audio": str(recon_path) if recon_path.exists() else "",
            "plot_path": str(wht_png) if wht_png.exists() else "",
            "matrix_csv": str(wht_db_path),
            "mse": math.nan,
            "rmse": math.nan,
            "max_abs": math.nan,
            "snr_db": math.nan,
            "energy_topk": energy_compaction_top_k(wht_db, args.topk_ratio),
            "peak_ratio": peak_to_total_ratio(wht_db),
            "sparsity_ratio_db60": sparsity_ratio(wht_db, -60.0),
        }

        if original_path and recon_path.exists():
            try:
                x, sr_x = read_audio(original_path)
                y, sr_y = read_audio(recon_path)
                if sr_x == sr_y:
                    row_wht.update(compute_audio_metrics(x, y))
            except Exception as e:
                print(f"[WARN] {prefix_name}: {e}")

        rows.append(row_wht)

        if stft_db_path.exists():
            stft_db = read_matrix_csv(stft_db_path)
            rows.append({
                "file_name": file_stem,
                "group": group,
                "transform": "stft",
                "order": "-",
                "original_audio": str(original_path) if original_path else "",
                "reconstructed_audio": "",
                "plot_path": str(stft_png) if stft_png.exists() else "",
                "matrix_csv": str(stft_db_path),
                "mse": math.nan,
                "rmse": math.nan,
                "max_abs": math.nan,
                "snr_db": math.nan,
                "energy_topk": energy_compaction_top_k(stft_db, args.topk_ratio),
                "peak_ratio": peak_to_total_ratio(stft_db),
                "sparsity_ratio_db60": sparsity_ratio(stft_db, -60.0),
            })

    out_csv = results_dir / "results_detailed.csv"
    with open(out_csv, "w", newline="", encoding="utf-8") as f:
        writer = csv.DictWriter(
            f,
            fieldnames=[
                "file_name", "group", "transform", "order",
                "original_audio", "reconstructed_audio", "plot_path", "matrix_csv",
                "mse", "rmse", "max_abs", "snr_db",
                "energy_topk", "peak_ratio", "sparsity_ratio_db60",
            ],
        )
        writer.writeheader()
        writer.writerows(rows)

    print(f"Saved: {out_csv}")


if __name__ == "__main__":
    main()