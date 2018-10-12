#!/bin/bash

# Location of current script
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

docker  run  \
    -v `pwd`:`pwd` \
    -w `pwd` --rm  \
    --user $(id -u):$(id -g) \
    -it elpaso/mxe-qt5-builder \
    ${DIR}/build-mxe.sh
