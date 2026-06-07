#!/usr/bin/env bash
# Legacy wrapper for the Strata dev build script.
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
exec "${ROOT}/scripts/build-strata-app.sh" "$@"
