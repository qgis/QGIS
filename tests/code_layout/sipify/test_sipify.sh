#!/usr/bin/env bash

# This runs sipify on the demo header and checks output

srcdir=$(dirname $0)/../../

DIR=$(git -C ${srcdir} rev-parse --show-toplevel)

pushd ${DIR} > /dev/null || exit
for pyqt_version in 5 6; do

  if [[ $pyqt_version == 6 ]]; then
    IS_QT6="-qt6"
  fi

  outdiff=$(./scripts/sipify.py $IS_QT6 tests/code_layout/sipify/sipifyheader.h | diff tests/code_layout/sipify/sipifyheader.expected_pyqt$pyqt_version.sip -)

  if [[ $outdiff ]]; then
    echo " *** sipify.py did not output expected file"
    echo "$outdiff"
    popd > /dev/null || exit
    exit 1
  fi
done
popd > /dev/null || exit
