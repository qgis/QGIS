#!/usr/bin/env bash
# Shared Strata Cloud backend origins for local launch scripts.
#
# Override at runtime, e.g.:
#   STRATA_BACKEND_PROD=https://my-service-xyz.a.run.app ./scripts/run-strata-prod.sh
#   STRATA_PLAN_ENDPOINT=https://custom.example/ai/messages ./scripts/run-strata-dev.sh

STRATA_BACKEND_LOCAL="${STRATA_BACKEND_LOCAL:-http://localhost:3001}"
# Cloud Run: strata-be, project strata-dev-qgis (372580174147), region europe-west1.
STRATA_BACKEND_PROD="${STRATA_BACKEND_PROD:-https://strata-be-372580174147.europe-west1.run.app}"
