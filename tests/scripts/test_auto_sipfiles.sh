#!/usr/bin/env bash

set -e

DIR=$(git rev-parse --show-toplevel)

# GNU prefix command for mac os support (gsed, gsplit)
GP=
if [[ "$OSTYPE" =~ darwin* ]]; then
  GP=g
fi

pushd ${DIR} > /dev/null

EXCLUDE=$(cat ${DIR}/python/auto_sip.blacklist | tr '\n' '|' | ${GP}sed -e 's/|$//')
FILES=$( find src -iname "*.h" \( -path 'src/core/*' -or -path 'src/gui/*' -or -path 'src/analysis/*' \) -type f | tr -s '[[:blank:]]' '\n' | egrep -iv "$EXCLUDE" | tr ' ' '\n')

code=0
for header in $FILES; do
  sipfile=$(sed -E 's/src\/(.*)\.h/python\/\1.sip/' <<< $header)
  outdiff=$(./scripts/sipify.pl $header | diff $sipfile -)
  if [[ -n $outdiff ]]; then
    if [[ $code == 0 ]]; then
      echo "some sip files are not up to date:"
      code=1
    fi
    echo "$sipfile"
  fi
done

popd > /dev/null

exit $code
