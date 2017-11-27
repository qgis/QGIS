#!/usr/bin/env bash

# This runs sipify on the demo header and checks output

DIR=$(git rev-parse --show-toplevel)

pushd ${DIR} > /dev/null
outdiff=$(./scripts/sipify.pl tests/code_layout/sipifyheader.h | diff tests/code_layout/sipifyheader.expected.sip -)
popd > /dev/null

if [[ $outdiff ]]; then
  echo " *** sipify.pl did not output expected file"
  echo "$outdiff"
  exit 1
fi
