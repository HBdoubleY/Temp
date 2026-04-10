#!/usr/bin/env bash
set -euo pipefail

# Usage:
#   bash perf_run_matrix.sh /path/to/run_binary_or_script
# Example:
#   bash perf_run_matrix.sh "/opt/work/app/run1.config"

if [[ $# -lt 1 ]]; then
  echo "usage: $0 <run-entry>"
  exit 1
fi

RUN_ENTRY="$1"
OUT_DIR="${2:-./perf_logs}"
mkdir -p "$OUT_DIR"

run_case() {
  local case_name="$1"
  local fps="$2"
  local res="$3"
  local log_file="$OUT_DIR/${case_name}.log"
  echo "[perf] run case=${case_name} fps=${fps} res=${res} -> ${log_file}"
  CP_PERF_ENABLE=1 \
  CP_PERF_DEEP=1 \
  CP_PERF_SAMPLE_N=5 \
  CP_PERF_WARN_US=20000 \
  ZLINK_SESSION_FPS="${fps}" \
  ZLINK_SESSION_RES="${res}" \
  sh -c "$RUN_ENTRY" >"${log_file}" 2>&1
}

# Baseline
run_case "baseline_1440x720_fps20" "20" "1440x720"

# Phase A
run_case "phaseA_720x360_fps20" "20" "720x360"

# A + B + C (代码已生效，仍使用同分辨率用于对比)
run_case "phaseABC_720x360_fps20" "20" "720x360"

echo "[perf] done. logs in ${OUT_DIR}"
