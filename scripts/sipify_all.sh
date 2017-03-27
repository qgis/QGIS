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

for header in $FILES; do
  echo "$header"
  sipfile=$(sed -E 's/src\/(.*)\.h/python\/\1.sip/' <<< $header)
  ./scripts/sipify.pl $header > $sipfile
done

popd > /dev/null
