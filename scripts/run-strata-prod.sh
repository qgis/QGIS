#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
# shellcheck source=scripts/strata-backend-endpoints.sh
source "${ROOT}/scripts/strata-backend-endpoints.sh"

export STRATA_PLAN_ENDPOINT="${STRATA_PLAN_ENDPOINT:-${STRATA_BACKEND_PROD}/ai/messages}"
exec "${ROOT}/scripts/_run-strata-app.sh" "$@"
