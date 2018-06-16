#!/usr/bin/env bash

# This test checks for use of deprecated/outdated methods and suggests their replacement

declare -a KEYWORDS=()
declare -a HINTS=()

KEYWORDS[0]="\-DBL_MAX"
HINTS[0]="Use the type-safe method std::numeric_limits<double>::lowest() instead"

KEYWORDS[1]="DBL_MAX"
HINTS[1]="Use the type-safe method std::numeric_limits<double>::max() instead"

KEYWORDS[2]="DBL_MIN"
HINTS[2]="Use the type-safe method std::numeric_limits<double>::min() instead (but be careful - maybe you actually want lowest!!)"

KEYWORDS[3]="DBL_EPSILON"
HINTS[3]="Use the type-safe method std::numeric_limits<double>::epsilon() instead"

KEYWORDS[4]="INT_MIN"
HINTS[4]="Use the type-safe method std::numeric_limits<int>::min() instead"

KEYWORDS[5]="INT_MAX"
HINTS[5]="Use the type-safe method std::numeric_limits<int>::max() instead"

KEYWORDS[6]="\bqMin("
HINTS[6]="Use std::min instead"

KEYWORDS[7]="\bqMax("
HINTS[7]="Use std::max instead"

KEYWORDS[8]="\bqAbs("
HINTS[8]="Use std::fabs instead"

KEYWORDS[9]="\bqRound("
HINTS[9]="Use std::round instead"

KEYWORDS[10]="\bqSort("
HINTS[10]="Use std::sort instead"

RES=
DIR=$(git rev-parse --show-toplevel)

pushd "${DIR}" > /dev/null || exit

for i in "${!KEYWORDS[@]}"
do
  FOUND=$(git grep "${KEYWORDS[$i]}" -- 'src/*.h' 'src/*.cpp' -- ':!*qtermwidget*')

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

