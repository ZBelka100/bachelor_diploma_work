#!/usr/bin/env bash
set -euo pipefail

mkdir -p data/out data/plots

# clean
for input in data/raw/librispeech_subset_noisy/clean/*.wav; do
  stem="$(basename "$input" .wav)"
  out_prefix="data/out/speechclean_${stem}"

  ./build/app \
    --input "$input" \
    --out "$out_prefix" \
    --frame 512 \
    --hop 128 \
    --window sqrt-hann \
    --order sequency \
    --do-stft 1 \
    --reconstruct "${out_prefix}_recon.wav"

  python3 scripts/plot_pseudospectrum.py --prefix "$out_prefix"
done

# noisy
for input in data/raw/librispeech_subset_noisy/snr*/*.wav; do
  base="$(basename "$input" .wav)"
  snr_tag="$(basename "$(dirname "$input")")"
  out_prefix="data/out/speech_${snr_tag}_${base}"

  ./build/app \
    --input "$input" \
    --out "$out_prefix" \
    --frame 512 \
    --hop 128 \
    --window sqrt-hann \
    --order sequency \
    --do-stft 1 \
    --reconstruct "${out_prefix}_recon.wav"

  python3 scripts/plot_pseudospectrum.py --prefix "$out_prefix"
done