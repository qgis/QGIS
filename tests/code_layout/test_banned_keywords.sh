#!/usr/bin/env bash

# This test checks for use of deprecated/outdated methods and suggests their replacement

declare -a KEYWORDS=()
declare -a HINTS=()

KEYWORDS[0]="DBL_MAX"
HINTS[0]="Use the type-safe method std::numeric_limits<double>::max() instead"

KEYWORDS[1]="DBL_MIN"
HINTS[1]="Use the type-safe method std::numeric_limits<double>::lowest() instead"

KEYWORDS[2]="DBL_EPSILON"
HINTS[2]="Use the type-safe method std::numeric_limits<double>::epsilon() instead"

RES=
DIR=$(git rev-parse --show-toplevel)

pushd "${DIR}" > /dev/null || exit

for i in "${!KEYWORDS[@]}"
do
  FOUND=$(git grep "${KEYWORDS[$i]}" -- 'src/*.h' 'src/*.cpp')

  if [[  ${FOUND} ]]; then
    echo "Found source files with banned keyword: ${KEYWORDS[$i]}!"
    echo " -> ${HINTS[$i]}"
    echo
    echo "${FOUND}"
    echo
    RES=1
  fi

done

popd > /dev/null || exit

if [ $RES ]; then
  echo " *** Found banned keywords"
  exit 1
fi

