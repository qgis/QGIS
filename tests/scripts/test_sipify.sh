#!/usr/bin/env bash

# This runs sipify on the demo header and checks output

DIR=$(git rev-parse --show-toplevel)

pushd ${DIR}
outdiff=$(./scripts/sipify.pl tests/scripts/sipifyheader.h | diff tests/scripts/sipifyheader.expected.sip -)
popd

if [[ $outdiff ]]; then
  echo " *** sipify.pl did not output expected file"
  echo "$outdiff"
  exit 1
fi
