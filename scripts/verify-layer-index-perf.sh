#!/usr/bin/env bash
# Compare Strata layer-load responsiveness with AI layer indexing on vs off.
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

if [[ "$(uname -s)" == "Darwin" ]]; then
  PROFILE="${HOME}/Library/Application Support/QGIS/QGIS4/profiles/default"
else
  PROFILE="${HOME}/.local/share/QGIS/QGIS4/profiles/default"
fi

echo "Layer load performance check"
echo "============================"
echo
echo "1. Baseline (indexing ON — default when local embeddings are available)"
echo "   ./scripts/run-strata-dev.sh"
echo "   Open a project with several layers or add a GPKG/shapefile."
echo "   Note time until the UI is responsive and whether Task Manager shows"
echo "   sequential 'Index AI layers' tasks."
echo
echo "2. Indexing OFF (isolates QGIS load cost from Strata AI indexing)"
echo "   AI Assistant → settings → disable automatic layer indexing"
echo "   (setting key: strata/index/enable_layer_indexing)"
echo "   Repeat the same open/add steps and compare."
echo
echo "3. Optional: skip Python plugins to measure QGIS-only startup"
echo "   ./scripts/run-strata-dev.sh -- -P"
echo
echo "4. Optional: remove stale SVG paths that can hang symbol lookup"
echo "   ${ROOT}/scripts/clean-stale-svg-paths.sh"
echo
echo "Profile directory: ${PROFILE}"

if [[ -d "${PROFILE}" ]]; then
  python3 - "${PROFILE}" <<'PY'
import sys
from pathlib import Path

profile = Path(sys.argv[1])
keys = ("strata/index/enable_layer_indexing", "strata/index/automatic")
found = False
for ini in list(profile.glob("QGIS*.ini")) + list((profile / "QGIS").glob("QGIS*.ini")):
    text = ini.read_text(encoding="utf-8", errors="replace")
    for key in keys:
        for line in text.splitlines():
            if line.startswith(key + "="):
                print(f"Current {ini.name}: {line}")
                found = True
if not found:
    print("No explicit strata/index/* overrides in profile (defaults apply).")
PY
fi
