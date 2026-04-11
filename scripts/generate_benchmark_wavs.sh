#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"

GENERATOR="${ROOT}/scripts/generate_machine_noise.py"
OUT_DIR="${ROOT}/data/wav/benchmark_lengths"
FS="${1:-16000}"

DURS=(1 2 5 10 20 30 60)

if [[ ! -f "${GENERATOR}" ]]; then
  echo "Ошибка: генератор не найден: ${GENERATOR}" >&2
  exit 1
fi

mkdir -p "${OUT_DIR}"

for dur in "${DURS[@]}"; do
  TMP_DIR="${OUT_DIR}/tmp_${dur}s"
  mkdir -p "${TMP_DIR}"

  echo "==> Генерация dur=${dur}s"
  python3 "${GENERATOR}" \
    --out-dir "${TMP_DIR}" \
    --fs "${FS}" \
    --dur "${dur}" \
    --seed 42

  mv "${TMP_DIR}/machine_hum.wav"      "${OUT_DIR}/machine_hum_${dur}s.wav"
  mv "${TMP_DIR}/rotating_machine.wav" "${OUT_DIR}/rotating_machine_${dur}s.wav"
  mv "${TMP_DIR}/faulty_machine.wav"   "${OUT_DIR}/faulty_machine_${dur}s.wav"
  mv "${TMP_DIR}/impulse_train.wav"    "${OUT_DIR}/impulse_train_${dur}s.wav"

  rmdir "${TMP_DIR}"
done

echo "==> Готово"
echo "Файлы сохранены в: ${OUT_DIR}"