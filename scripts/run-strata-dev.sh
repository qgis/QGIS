#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD="${STRATA_BUILD_DIR:-${GEOAI_BUILD_DIR:-${ROOT}/build}}"
BUILD_OUTPUT="${BUILD}/output"

if [[ -z "${PYTHONHOME:-}" ]]; then
  PYTHON_EXECUTABLE="$(awk -F= '/^Python_EXECUTABLE:FILEPATH=/{print $2; exit}' "${BUILD}/CMakeCache.txt" 2>/dev/null || true)"
  if [[ -z "${PYTHON_EXECUTABLE}" ]]; then
    PYTHON_EXECUTABLE="$(command -v python3)"
  fi
  export PYTHONHOME="$("${PYTHON_EXECUTABLE}" -c 'import sys; print(sys.base_prefix)')"
fi

"${ROOT}/scripts/patch-macos-bundle.sh" >/dev/null

exec "${BUILD_OUTPUT}/Contents/MacOS/Strata" "$@"
