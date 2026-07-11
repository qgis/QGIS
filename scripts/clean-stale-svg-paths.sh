#!/usr/bin/env bash
# Remove missing or stale SVG search paths from the default QGIS/Strata profile.
# Stale dev paths (old build trees, unmounted volumes) can slow symbol resolution.
set -euo pipefail

PROFILE="${1:-${QGIS_PROFILE_PATH:-}}"

if [[ -z "${PROFILE}" ]]; then
  if [[ "$(uname -s)" == "Darwin" ]]; then
    PROFILE="${HOME}/Library/Application Support/Strata/Strata/profiles/default"
    LEGACY_PROFILE="${HOME}/Library/Application Support/QGIS/QGIS4/profiles/default"
  else
    PROFILE="${HOME}/.local/share/Strata/Strata/profiles/default"
    LEGACY_PROFILE="${HOME}/.local/share/QGIS/QGIS4/profiles/default"
  fi

  if [[ ! -d "${PROFILE}" && -d "${LEGACY_PROFILE}" ]]; then
    PROFILE="${LEGACY_PROFILE}"
  fi
fi

if [[ ! -d "${PROFILE}" ]]; then
  echo "Profile directory not found: ${PROFILE}" >&2
  exit 1
fi

python3 - "${PROFILE}" <<'PY'
import re
import sys
from pathlib import Path

profile = Path(sys.argv[1])
# QSettings stores the INI under the organisation subdir (e.g. qgis.org/QGIS4.ini)
# on most installs; older/custom layouts keep it directly under the profile or in a
# QGIS/ subdir. Probe all known locations.
ini_candidates = sorted(
    {
        *profile.glob("QGIS*.ini"),
        *profile.glob("QGIS/QGIS*.ini"),
        *profile.glob("qgis.org/QGIS*.ini"),
    }
)
if not ini_candidates:
    print(f"No QGIS*.ini found under {profile}", file=sys.stderr)
    sys.exit(1)

stale_markers = (
    "GeoAI-Desktop",
    "QGIS_AI",
    "LLM_MODELS",
)

# QGIS injects built-in marker search paths (e.g. "/svg/") that it resolves at
# runtime relative to its install; they are not real on-disk directories, so the
# exists() heuristic must never classify them as stale.
builtin_svg_paths = ("/svg/",)

def is_stale(path: str) -> bool:
    if path in builtin_svg_paths:
        return False
    if any(marker in path for marker in stale_markers):
        return True
    if path.startswith("/Volumes/") and not Path(path).exists():
        return True
    return not Path(path).exists()

changed = 0
for ini_path in ini_candidates:
    # surrogateescape round-trips any non-UTF8 byte losslessly, so unrelated
    # settings are never corrupted on write-back.
    text = ini_path.read_text(encoding="utf-8", errors="surrogateescape")
    lines = text.splitlines(keepends=True)
    out = []
    i = 0
    file_changed = False

    while i < len(lines):
        line = lines[i]
        if re.match(r"^searchPathsForSVG\\size=", line):
            try:
                size = int(line.split("=", 1)[1].strip())
            except ValueError:
                out.append(line)
                i += 1
                continue
            entries = []
            consumed = 1  # the size= line itself
            for j in range(1, size + 1):
                idx = i + j
                # Stop if the declared size overruns the file or the numbered
                # entries end early; the remaining lines are then reprocessed
                # normally by the outer loop instead of being swallowed.
                if idx >= len(lines) or not lines[idx].startswith(f"searchPathsForSVG\\{j}="):
                    break
                consumed += 1
                value = lines[idx].split("=", 1)[1].strip()
                if is_stale(value):
                    file_changed = True
                    print(f"Removing stale SVG path from {ini_path.name}: {value}")
                else:
                    entries.append(value)
            if entries:
                out.append(f"searchPathsForSVG\\size={len(entries)}\n")
                for idx, value in enumerate(entries, start=1):
                    out.append(f"searchPathsForSVG\\{idx}={value}\n")
            else:
                out.append("searchPathsForSVG\\size=0\n")
                file_changed = True
            i += consumed
            continue

        if line.startswith("searchPathsForSVG=") and "\\" not in line.split("=", 1)[0]:
            raw = line.split("=", 1)[1].strip()
            parts = [p.strip() for p in raw.split(",") if p.strip()]
            kept = [p for p in parts if not is_stale(p)]
            if kept != parts:
                file_changed = True
                for removed in parts:
                    if removed not in kept:
                        print(f"Removing stale SVG path from {ini_path.name}: {removed}")
            if kept:
                out.append("searchPathsForSVG=" + ", ".join(kept) + "\n")
            else:
                out.append("searchPathsForSVG=\n")
            i += 1
            continue

        out.append(line)
        i += 1

    if file_changed:
        ini_path.write_text("".join(out), encoding="utf-8", errors="surrogateescape")
        changed += 1

if changed:
    print(f"Updated {changed} profile file(s). Restart Strata for changes to apply.")
else:
    print("No stale SVG paths found.")
PY
