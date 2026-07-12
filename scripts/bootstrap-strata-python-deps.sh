#!/usr/bin/env bash
# Install Python runtime packages needed by core QGIS plugins in a dev build.
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD="${STRATA_BUILD_DIR:-${GEOAI_BUILD_DIR:-${ROOT}/build}}"
PYTHON_OUTPUT="${BUILD}/output/python"
PYTHON_EXECUTABLE="${STRATA_PYTHON_EXECUTABLE:-${GEOAI_PYTHON_EXECUTABLE:-}}"

if [[ ! -d "${BUILD}" ]]; then
  echo "Build directory not found: ${BUILD}" >&2
  exit 1
fi

if [[ ! -d "${PYTHON_OUTPUT}" ]]; then
  echo "Python output directory not found: ${PYTHON_OUTPUT}" >&2
  echo "Build Strata first with: STRATA_BUILD_DIR=${BUILD} ${ROOT}/scripts/build-strata-app.sh" >&2
  exit 1
fi

if [[ -z "${PYTHON_EXECUTABLE}" && -f "${BUILD}/CMakeCache.txt" ]]; then
  PYTHON_EXECUTABLE="$(awk -F= '/^Python_EXECUTABLE:[^=]*=/ { print $2; exit }' "${BUILD}/CMakeCache.txt")"
fi

if [[ -z "${PYTHON_EXECUTABLE}" ]]; then
  PYTHON_EXECUTABLE="$(command -v python3 || true)"
fi

if [[ -z "${PYTHON_EXECUTABLE}" || ! -x "${PYTHON_EXECUTABLE}" ]]; then
  echo "Python executable not found. Set STRATA_PYTHON_EXECUTABLE=/path/to/python." >&2
  exit 1
fi

echo "Installing Python dev runtime packages into: ${PYTHON_OUTPUT}"
shopt -s nullglob
for package_path in \
  "${PYTHON_OUTPUT}"/packaging \
  "${PYTHON_OUTPUT}"/packaging-*.dist-info \
  "${PYTHON_OUTPUT}"/jinja2 \
  "${PYTHON_OUTPUT}"/jinja2-*.dist-info \
  "${PYTHON_OUTPUT}"/markupsafe \
  "${PYTHON_OUTPUT}"/markupsafe-*.dist-info
do
  rm -rf "${package_path}"
done

"${PYTHON_EXECUTABLE}" -m pip install --upgrade --target "${PYTHON_OUTPUT}" \
  packaging
"${PYTHON_EXECUTABLE}" -m pip install --upgrade --no-deps --target "${PYTHON_OUTPUT}" \
  jinja2
"${PYTHON_EXECUTABLE}" -m pip install --upgrade --no-binary MarkupSafe --target "${PYTHON_OUTPUT}" \
  MarkupSafe

PYTHONPATH="${PYTHON_OUTPUT}${PYTHONPATH:+:${PYTHONPATH}}" "${PYTHON_EXECUTABLE}" - <<'PY'
import jinja2
import markupsafe
import packaging

print("Python runtime deps OK:")
print("  packaging", packaging.__version__)
print("  jinja2", jinja2.__version__)
print("  MarkupSafe", markupsafe.__version__)
PY
