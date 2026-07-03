#!/usr/bin/env bash
# Shared Strata Cloud backend origins for local launch scripts.
#
# Override at runtime, e.g.:
#   STRATA_BACKEND_PROD=https://my-service-xyz.a.run.app ./scripts/run-strata-prod.sh
#   STRATA_PLAN_ENDPOINT=https://custom.example/ai/messages ./scripts/run-strata-dev.sh

STRATA_BACKEND_LOCAL="${STRATA_BACKEND_LOCAL:-http://localhost:3001}"
STRATA_BACKEND_PROD="${STRATA_BACKEND_PROD:-https://palasor-be-5f3ydhgs4q-ew.a.run.app}"
