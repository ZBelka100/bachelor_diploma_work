#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"

INPUT_DIR="${1:-${ROOT}/data/wav/benchmark_lengths}"
ITERS="${2:-100}"
FFT_IMPL="${3:-custom}"   # custom | fftw

case "${FFT_IMPL}" in
  custom)
    USE_FFTW=OFF
    BUILD_DIR="${ROOT}/build_bench_custom"
    CSV_OUT="${ROOT}/data/reports/summary/benchmark_results_custom_fft.csv"
    ;;
  fftw)
    USE_FFTW=ON
    BUILD_DIR="${ROOT}/build_bench_fftw"
    CSV_OUT="${ROOT}/data/reports/summary/benchmark_results_fftw.csv"
    ;;
  *)
    echo "Ошибка: FFT_IMPL должен быть custom или fftw" >&2
    exit 1
    ;;
esac

BENCH_BIN="${BUILD_DIR}/benchmark_app"

FRAMES=(256 512 1024 2048)
WINDOWS=("rect" "hann" "sqrt-hann")
ORDERINGS=("hadamard" "sequency" "dyadic")

mkdir -p \
  "${ROOT}/data/reports/summary"

if [[ ! -d "${INPUT_DIR}" ]]; then
  echo "Ошибка: входная директория не найдена: ${INPUT_DIR}" >&2
  exit 1
fi

echo "==> Удаляю старую сборку: ${BUILD_DIR}"
rm -rf "${BUILD_DIR}"

cmake -S "${ROOT}" -B "${BUILD_DIR}" -DCMAKE_BUILD_TYPE=Release -DUSE_FFTW="${USE_FFTW}"
cmake --build "${BUILD_DIR}" -j

if [[ ! -x "${BENCH_BIN}" ]]; then
  echo "Ошибка: не найден бинарник ${BENCH_BIN}" >&2
  exit 1
fi

rm -f "${CSV_OUT}"

find "${INPUT_DIR}" -type f -name "*.wav" | sort | while read -r wav; do
  echo "==> FILE: ${wav} [fft=${FFT_IMPL}]"

  for frame in "${FRAMES[@]}"; do
    hop=$((frame / 4))

    for window in "${WINDOWS[@]}"; do
      for ordering in "${ORDERINGS[@]}"; do
        echo "   frame=${frame}, hop=${hop}, window=${window}, ordering=${ordering}"

        "${BENCH_BIN}" \
          --input "${wav}" \
          --csv "${CSV_OUT}" \
          --frame "${frame}" \
          --hop "${hop}" \
          --iters "${ITERS}" \
          --window "${window}" \
          --order "${ordering}" \
          --fft-impl "${FFT_IMPL}"
      done
    done
  done
done

echo
echo "==> Готово"
echo "CSV: ${CSV_OUT}"