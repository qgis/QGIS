#!/usr/bin/env bash
DIR=$(git rev-parse --show-toplevel)
cd $1 || exit
FOLDER=linux
ctest -E "$(cat ${DIR}/.ci/travis/${FOLDER}/blacklist.txt | sed -r '/^(#.*?)?$/d' | paste -sd '|' -)" --output-on-failure
cd $DIR || exit
