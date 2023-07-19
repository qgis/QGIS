#!/usr/bin/env bash

# This test checks for use of non-compliant class names

declare -a KEYWORDS=()
declare -a HINTS=()

KEYWORDS[0]="^\s*class[^:]*Qgs\S*3d"
HINTS[0]="Use '3D' capitalisation in class names instead of '3d'"

RES=
DIR=$(git rev-parse --show-toplevel)

pushd "${DIR}" > /dev/null || exit

for i in "${!KEYWORDS[@]}"
do
  FOUND=$(git grep "${KEYWORDS[$i]}" -- 'src/*.h' 'src/*.cpp' | sed -n 's/.*\(Qgs\w*\).*/\1/p' | sort -u)

  if [[  ${FOUND} ]]; then
    echo "Found classes with non-standard names!"
    echo " -> ${HINTS[$i]}"
    echo
    echo "${FOUND}"
    echo
    RES=1
  fi

done

popd > /dev/null || exit

if [ $RES ]; then
  echo " *** Found non-compliant class names"
  exit 1
fi

