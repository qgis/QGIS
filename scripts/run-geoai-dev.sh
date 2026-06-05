#!/usr/bin/env bash
set -euo pipefail

BUILD_OUTPUT="/Volumes/LLM_MODELS/QGIS_AI/build/output"
export PYTHONHOME="/opt/homebrew/opt/python@3.14/Frameworks/Python.framework/Versions/3.14"

exec "${BUILD_OUTPUT}/Contents/MacOS/qgis" "$@"
