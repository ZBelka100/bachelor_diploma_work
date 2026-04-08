#!/usr/bin/env bash
set -euo pipefail

INPUT="${1:-data/wav/1-119125-A-45.wav}"
OUT_PREFIX="${2:-data/out/washing_machine}"

mkdir -p build data/out data/plots

cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DUSE_FFTW=OFF
cmake --build build -j

./build/app \
  --input "${INPUT}" \
  --out "${OUT_PREFIX}" \
  --frame 1024 \
  --hop 256 \
  --window sqrt-hann \
  --order sequency \
  --do-stft 1 \
  --reconstruct "${OUT_PREFIX}_recon.wav"

python3 scripts/plot_pseudospectrum.py --prefix "${OUT_PREFIX}"