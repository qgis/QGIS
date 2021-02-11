#!/bin/bash

# Location of current script
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

docker  run  \
    -v $(pwd):$(pwd) \
    -w $(pwd) --rm  \
    --user $(id -u):$(id -g) \
    --env CCACHE_DIR=${CCACHE_DIR} \
    mxe-qt5-builder:current \
    ${DIR}/build-mxe.sh
