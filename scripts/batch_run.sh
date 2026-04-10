#!/usr/bin/env bash
set -euo pipefail

cd "$(dirname "$0")/.."

mkdir -p build data/out data/plots logs

cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DUSE_FFTW=OFF
cmake --build build -j

process_dir() {
  local input_dir="$1"
  local group="$2"

  while IFS= read -r -d '' input; do
    base="$(basename "$input")"
    stem="${base%.*}"
    out_prefix="data/out/${group}_${stem}"

    echo "========================================"
    echo "Processing: $input"
    echo "Output:     $out_prefix"
    echo "========================================"

    ./build/app \
      --input "$input" \
      --out "$out_prefix" \
      --frame 1024 \
      --hop 256 \
      --window sqrt-hann \
      --order sequency \
      --do-stft 1 \
      --reconstruct "${out_prefix}_recon.wav" \
      </dev/null

    python3 scripts/plot_pseudospectrum.py \
      --prefix "$out_prefix" \
      --plots-dir data/plots \
      </dev/null

  done < <(find "$input_dir" -maxdepth 1 -type f -name "*.wav" -print0 | sort -z)
}

process_dir "data/raw/librispeech_subset" "speech"
process_dir "data/raw/mimii_subset" "machine"
process_dir "data/raw/fsdd_subset" "fsdd"
process_dir "data/raw/esc-50" "events"