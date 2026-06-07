#!/usr/bin/env bash
# Legacy wrapper for the Strata dev launcher.
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
exec "${ROOT}/scripts/run-strata-dev.sh" "$@"
