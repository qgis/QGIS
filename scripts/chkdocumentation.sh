#!/usr/bin/env bash
# shellcheck disable=SC2164
PROG_NAME=$(basename $0)

set -E
set -o functrace
function handle_error {
    local retval=$?
    local line=${last_lineno:-$1}
    echo "Failed at $PROG_NAME:$line $BASH_COMMAND"
    exit $retval
}
trap 'handle_error $LINENO ${BASH_LINENO[@]}' ERR

SOURCE_DIR=$(dirname $0)/..
SOURCE_DIR=$(realpath $SOURCE_DIR)
DOC_SCRIPT=$SOURCE_DIR/scripts/documentation_check

echo "== Build docker image"
docker build -t qgis/doc_check:latest -f "$DOC_SCRIPT/Dockerfile" "$DOC_SCRIPT/"

echo "== Run docker image"
docker run --rm -ti -v "$SOURCE_DIR:/qgis" qgis/doc_check:latest

echo "== Done!"
