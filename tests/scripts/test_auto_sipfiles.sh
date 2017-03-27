#!/usr/bin/env bash

DIR=$(git rev-parse --show-toplevel)

code=0

while read -r line; do
  header="src/$line"
  sipfile=$(sed -E 's/(.*)\.h/python\/\1.sip/' <<< $line)
  outdiff=$(${DIR}/scripts/sipify.pl ${DIR}/$header | diff ${DIR}/$sipfile -)
  if [[ -n $outdiff ]]; then
    if [[ $code == 0 ]]; then
      echo "some sip files are not up to date:"
      code=1
    fi
    echo "$sipfile"
  fi
done < ${DIR}/python/auto_sipfiles.txt

exit $code
