#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"

CSV_INPUT="${ROOT}/data/reports/summary/sample_inputs.csv"
HOP_MODE="${1:-quarter}"   # quarter | half
ORDER="${2:-sequency}"
WINDOWS=(rect hann sqrt-hann)
FRAMES=(256 512 1024 2048)

if [[ ! -f "${CSV_INPUT}" ]]; then
  echo "Ошибка: не найден ${CSV_INPUT}" >&2
  exit 1
fi

mkdir -p \
  "${ROOT}/build" \
  "${ROOT}/data/out/sweeps/window_size" \
  "${ROOT}/data/plots/sweeps/window_size" \
  "${ROOT}/data/reports/summary"

cmake -S "${ROOT}" -B "${ROOT}/build" -DCMAKE_BUILD_TYPE=Release -DUSE_FFTW=OFF
cmake --build "${ROOT}/build" -j

MASTER_SUMMARY_CSV="${ROOT}/data/reports/summary/window_sweep_runs_all_windows.csv"
echo "dataset,index,file_name,frame,hop,window,order,out_prefix,plot_dir,status" > "${MASTER_SUMMARY_CSV}"

calc_hop() {
  local frame="$1"
  local mode="$2"
  case "$mode" in
    quarter) echo $((frame / 4)) ;;
    half) echo $((frame / 2)) ;;
    *)
      echo "Ошибка: неизвестный режим hop: ${mode}" >&2
      exit 1
      ;;
  esac
}

run_one() {
  local dataset="$1"
  local index="$2"
  local file_name="$3"
  local full_path="$4"
  local window="$5"
  local summary_csv="$6"

  local stem="${file_name%.*}"

  for frame in "${FRAMES[@]}"; do
    local hop
    hop="$(calc_hop "${frame}" "${HOP_MODE}")"

    local config="N${frame}_H${hop}_${window}_${ORDER}"
    local out_dir="${ROOT}/data/out/sweeps/window_size/${window}/${dataset}/${stem}/${config}"
    local plot_dir="${ROOT}/data/plots/sweeps/window_size/${window}/${dataset}/${stem}/${config}"
    local out_prefix="${out_dir}/${stem}"

    mkdir -p "${out_dir}"

    echo
    echo "==> window=${window} dataset=${dataset} index=${index} file=${file_name} config=${config}"

    local status="ok"

    if ! "${ROOT}/build/app" \
      --input "${full_path}" \
      --out "${out_prefix}" \
      --frame "${frame}" \
      --hop "${hop}" \
      --window "${window}" \
      --order "${ORDER}" \
      --do-stft 1 \
      --reconstruct "${out_prefix}_recon.wav"; then
      echo "Ошибка: app завершился с ошибкой для ${file_name} (${config})" >&2
      status="app_failed"
      printf "%s,%s,%s,%s,%s,%s,%s,%s,%s,%s\n" \
        "${dataset}" "${index}" "${file_name}" "${frame}" "${hop}" "${window}" "${ORDER}" "${out_prefix}" "${plot_dir}" "${status}" \
        >> "${summary_csv}"
      printf "%s,%s,%s,%s,%s,%s,%s,%s,%s,%s\n" \
        "${dataset}" "${index}" "${file_name}" "${frame}" "${hop}" "${window}" "${ORDER}" "${out_prefix}" "${plot_dir}" "${status}" \
        >> "${MASTER_SUMMARY_CSV}"
      continue
    fi

    if ! python3 "${ROOT}/scripts/plot_pseudospectrum.py" \
      --prefix "${out_prefix}" \
      --plots-dir "${plot_dir}" \
      --vmin -120 \
      --vmax 0; then
      echo "Ошибка: plot_pseudospectrum.py завершился с ошибкой для ${file_name} (${config})" >&2
      status="plot_failed"
    fi

    printf "%s,%s,%s,%s,%s,%s,%s,%s,%s,%s\n" \
      "${dataset}" "${index}" "${file_name}" "${frame}" "${hop}" "${window}" "${ORDER}" "${out_prefix}" "${plot_dir}" "${status}" \
      >> "${summary_csv}"
    printf "%s,%s,%s,%s,%s,%s,%s,%s,%s,%s\n" \
      "${dataset}" "${index}" "${file_name}" "${frame}" "${hop}" "${window}" "${ORDER}" "${out_prefix}" "${plot_dir}" "${status}" \
      >> "${MASTER_SUMMARY_CSV}"
  done
}

for window in "${WINDOWS[@]}"; do
  summary_csv="${ROOT}/data/reports/summary/window_sweep_runs_${window}.csv"
  echo "dataset,index,file_name,frame,hop,window,order,out_prefix,plot_dir,status" > "${summary_csv}"

  tail -n +2 "${CSV_INPUT}" | while IFS=, read -r dataset index file_name full_path; do
    [[ -n "${dataset}" ]] || continue
    [[ -f "${full_path}" ]] || {
      echo "Пропуск: файл не найден: ${full_path}" >&2
      continue
    }
    run_one "${dataset}" "${index}" "${file_name}" "${full_path}" "${window}" "${summary_csv}"
  done
done

echo
echo "Готово."
echo "Общая сводка: ${MASTER_SUMMARY_CSV}"
echo "Сводки по типам окна:"
for window in "${WINDOWS[@]}"; do
  echo "  ${ROOT}/data/reports/summary/window_sweep_runs_${window}.csv"
done