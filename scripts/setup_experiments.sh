#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"

RAW_DIR="${ROOT}/data/raw"
REPORT_DIR="${ROOT}/data/reports/summary"
CSV_OUT="${REPORT_DIR}/sample_inputs.csv"

if [[ ! -d "${RAW_DIR}" ]]; then
  echo "Ошибка: не найдена директория ${RAW_DIR}" >&2
  exit 1
fi

mkdir -p \
  "${ROOT}/build" \
  "${ROOT}/data/out/single" \
  "${ROOT}/data/out/sweeps/window_size" \
  "${ROOT}/data/out/sweeps/window_type" \
  "${ROOT}/data/plots/single" \
  "${ROOT}/data/plots/sweeps/window_size" \
  "${ROOT}/data/plots/sweeps/window_type" \
  "${ROOT}/data/reports/summary" \
  "${ROOT}/results"

echo "dataset,index,file_name,full_path" > "${CSV_OUT}"

find_audio_files() {
  local dir="$1"
  find "$dir" -type f \( \
    -iname "*.wav" -o \
    -iname "*.flac" -o \
    -iname "*.mp3" -o \
    -iname "*.ogg" -o \
    -iname "*.m4a" \
  \) | sort
}

echo "Корень проекта: ${ROOT}"
echo
echo "Выбраны по 2 аудиофайла из каждой директории data/raw:"
echo

while IFS= read -r dir; do
  [[ -d "${dir}" ]] || continue

  dir_name="$(basename "${dir}")"
  mapfile -t files < <(find_audio_files "${dir}" | head -n 2)

  if [[ ${#files[@]} -eq 0 ]]; then
    echo "${dir_name}: [аудиофайлы не найдены]"
    continue
  fi

  echo "${dir_name}:"
  for i in "${!files[@]}"; do
    idx=$((i + 1))
    base="$(basename "${files[$i]}")"
    echo "  ${idx}) ${base}"
    printf "%s,%d,%s,%s\n" "${dir_name}" "${idx}" "${base}" "${files[$i]}" >> "${CSV_OUT}"
  done
  echo
done < <(find "${RAW_DIR}" -mindepth 1 -maxdepth 1 -type d | sort)

echo "Сохранено:"
echo "  ${CSV_OUT}"