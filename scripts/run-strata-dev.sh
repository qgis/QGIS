#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD="${STRATA_BUILD_DIR:-${GEOAI_BUILD_DIR:-${ROOT}/build}}"
BUILD_OUTPUT="${BUILD}/output"
STRATA_BIN="${BUILD_OUTPUT}/Contents/MacOS/Strata"

if [[ ! -x "${STRATA_BIN}" ]]; then
  echo "Strata binary not found: ${STRATA_BIN}" >&2
  echo "Build first with: ${ROOT}/scripts/build-strata-app.sh" >&2
  exit 1
fi

# PYTHONHOME must match the Python used to build/link PyQGIS. A wrong value can leave
# the embedded interpreter in a broken state and break subprocess pip installs even
# when the Python console appears to work. Override explicitly with STRATA_PYTHONHOME.
if [[ -n "${STRATA_PYTHONHOME:-}" ]]; then
  export PYTHONHOME="${STRATA_PYTHONHOME}"
elif [[ -z "${PYTHONHOME:-}" ]]; then
  PYTHON_EXECUTABLE="$(
    awk -F= '/^Python_EXECUTABLE:/ { print $2; exit }' "${BUILD}/CMakeCache.txt" 2>/dev/null || true
  )"
  if [[ -n "${PYTHON_EXECUTABLE}" && -x "${PYTHON_EXECUTABLE}" ]]; then
    export PYTHONHOME="$("${PYTHON_EXECUTABLE}" -c 'import sys; print(sys.base_prefix)')"
  fi
fi

export PYTHONPATH="${BUILD_OUTPUT}/python${PYTHONPATH:+:${PYTHONPATH}}"

# Do not set QGIS_PREFIX_PATH here: it breaks macOS build-dir detection
# (qgisbuildpath.txt lives under Contents/MacOS, not under output/).

"${ROOT}/scripts/patch-macos-bundle.sh" >/dev/null

exec "${STRATA_BIN}" "$@"
