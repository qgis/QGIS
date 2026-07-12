#!/usr/bin/env bash
# Incremental Strata desktop build.
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD="${STRATA_BUILD_DIR:-${GEOAI_BUILD_DIR:-${ROOT}/build}}"
JOBS="${STRATA_BUILD_JOBS:-${GEOAI_BUILD_JOBS:-$(sysctl -n hw.ncpu)}}"
TARGET="${STRATA_BUILD_TARGET:-${GEOAI_BUILD_TARGET:-qgis_desktop}}"

if [[ ! -d "${BUILD}" ]]; then
  echo "Build directory not found: ${BUILD}" >&2
  exit 1
fi

if pgrep -x ninja >/dev/null 2>&1; then
  echo "Another ninja build is already running. Stop it first to avoid duplicate compiles." >&2
  exit 1
fi

cd "${BUILD}"
cmake --build . --target "${TARGET}" -j"${JOBS}"
"${ROOT}/scripts/patch-macos-bundle.sh"

echo "Build target '${TARGET}' finished. Launch with: ${ROOT}/scripts/run-strata-dev.sh"
