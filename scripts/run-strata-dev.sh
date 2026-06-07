#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_OUTPUT="${STRATA_BUILD_DIR:-${GEOAI_BUILD_DIR:-${ROOT}/build}}/output"
export PYTHONHOME="/opt/homebrew/opt/python@3.14/Frameworks/Python.framework/Versions/3.14"

"${ROOT}/scripts/patch-macos-bundle.sh" >/dev/null

exec "${BUILD_OUTPUT}/Contents/MacOS/Strata" "$@"
