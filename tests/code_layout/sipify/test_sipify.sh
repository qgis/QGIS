#!/usr/bin/env bash

# This runs sipify on the demo header and checks output

srcdir=$(dirname $0)/../../

DIR=$(git -C ${srcdir} rev-parse --show-toplevel)

pushd ${DIR} > /dev/null || exit
outdiff=$(./scripts/sipify.pl tests/code_layout/sipify/sipifyheader.h | diff tests/code_layout/sipify/sipifyheader.expected.sip -)
popd > /dev/null || exit

if [[ $outdiff ]]; then
  echo " *** sipify.pl did not output expected file"
  echo "$outdiff"
  exit 1
fi
