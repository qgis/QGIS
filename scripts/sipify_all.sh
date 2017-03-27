#!/usr/bin/env bash

DIR=$(git rev-parse --show-toplevel)

set -e

while read -r line; do
  header="src/$line"
  sipfile=$(sed -E 's/(.*)\.h/python\/\1.sip/' <<< $line)
  echo "$header"
  ${DIR}/scripts/sipify.pl ${DIR}/$header > ${DIR}/$sipfile
done < ${DIR}/python/auto_sipfiles.txt
