import argparse
import csv
import math
from collections import defaultdict
from pathlib import Path

NUMERIC_FIELDS = [
    "mse",
    "rmse",
    "max_abs",
    "snr_db",
    "energy_topk",
    "peak_ratio",
    "sparsity_ratio_db60",
]


def safe_float(x):
    try:
        v = float(x)
        if math.isnan(v):
            return None
        if math.isinf(v):
            return None
        return v
    except Exception:
        return None


def mean(values):
    vals = [v for v in values if v is not None]
    if not vals:
        return math.nan
    return sum(vals) / len(vals)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--input", default="results/results_detailed.csv")
    parser.add_argument("--output", default="results/results_summary.csv")
    args = parser.parse_args()

    grouped = defaultdict(list)

    with open(args.input, "r", encoding="utf-8") as f:
        reader = csv.DictReader(f)
        for row in reader:
            key = (row["group"], row["transform"], row["order"])
            grouped[key].append(row)

    out_rows = []
    for (group, transform, order), rows in grouped.items():
        out = {
            "group": group,
            "transform": transform,
            "order": order,
            "n": len(rows),
        }
        for field in NUMERIC_FIELDS:
            out[field + "_mean"] = mean([safe_float(r[field]) for r in rows])
        out_rows.append(out)

    out_rows.sort(key=lambda x: (x["group"], x["transform"], x["order"]))

    Path(args.output).parent.mkdir(parents=True, exist_ok=True)
    with open(args.output, "w", newline="", encoding="utf-8") as f:
        writer = csv.DictWriter(
            f,
            fieldnames=[
                "group", "transform", "order", "n",
                "mse_mean", "rmse_mean", "max_abs_mean", "snr_db_mean",
                "energy_topk_mean", "peak_ratio_mean", "sparsity_ratio_db60_mean",
            ],
        )
        writer.writeheader()
        writer.writerows(out_rows)

    print(f"Saved: {args.output}")


if __name__ == "__main__":
    main()