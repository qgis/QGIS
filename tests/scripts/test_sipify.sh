#!/usr/bin/env bash

# This runs sipify on the demo header and checks output

DIR=$(git rev-parse --show-toplevel)

outdiff=$(${DIR}/scripts/sipify.pl ${DIR}/tests/scripts/sipifyheader.h | diff ${DIR}/tests/scripts/sipifyheader.expected.sip -)

if [[ $outdiff ]]; then
  echo " *** sipify.pl did not output expected file"
  echo "$outdiff"
  exit 1
fi
