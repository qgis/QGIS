#!/usr/bin/env bash

# This test checks for use of QScrollArea in .ui files and suggests using QgsScrollArea instead

RES=
DIR=$(git rev-parse --show-toplevel)

pushd "${DIR}" > /dev/null || exit

FOUND=$(git grep "class=\"QScrollArea\"" -- 'src/*.ui' | grep --invert-match skip-keyword-check)

if [[  ${FOUND} ]]; then
  echo "Base QScrollArea class used in .ui file!"
  echo " -> Use QgsScrollArea class instead"
  echo
  echo "${FOUND}"
  echo
  RES=1
fi

FOUND=$(git grep "new QScrollArea" -- 'src' | grep --invert-match skip-keyword-check)

if [[  ${FOUND} ]]; then
  echo "Base QScrollArea class used in file!"
  echo " -> Use QgsScrollArea class instead"
  echo "    or mark with // skip-keyword-check"  echo
  echo "${FOUND}"
  echo
  RES=1
fi


popd > /dev/null || exit

if [ $RES ]; then
  echo " *** Found QScrollArea use in ui files"
  exit 1
fi

