#!/usr/bin/env bash
# Install Strata branding files into the dev bundle (Info.plist + icon).
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD="${STRATA_BUILD_DIR:-${GEOAI_BUILD_DIR:-${ROOT}/build}}"
OUTPUT="${BUILD}/output"
CONTENTS="${OUTPUT}/Contents"
RESOURCES="${CONTENTS}/Resources"

if [[ ! -d "${OUTPUT}" ]]; then
  echo "Build output not found: ${OUTPUT}" >&2
  exit 1
fi

INFO_SRC="${BUILD}/platform/macos/Info.plist"
ICON_SRC="${ROOT}/images/icons/mac/strata.icns"

if [[ ! -f "${INFO_SRC}" ]]; then
  echo "Run cmake configure first (missing ${INFO_SRC})." >&2
  exit 1
fi

mkdir -p "${RESOURCES}"
cp "${INFO_SRC}" "${CONTENTS}/Info.plist"
cp "${ICON_SRC}" "${RESOURCES}/strata.icns"

echo "Patched bundle:"
echo "  ${CONTENTS}/Info.plist"
echo "  ${RESOURCES}/strata.icns"
