from pathlib import Path

DIRS = [
    "data/raw/machine_normal",
    "data/raw/machine_anomaly",
    "data/raw/speech",
    "data/raw/events",
    "data/out",
    "data/plots",
    "results",
    "logs",
]

def main():
    for d in DIRS:
        Path(d).mkdir(parents=True, exist_ok=True)
        print(f"Created: {d}")

if __name__ == "__main__":
    main()