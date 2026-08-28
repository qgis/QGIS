#!/usr/bin/env bash

# This test checks for use of QFontComboBox in .ui files and suggests using QgsFontComboBox instead

RES=
DIR=$(git rev-parse --show-toplevel)

pushd "${DIR}" > /dev/null || exit

FOUND=$(git grep "class=\"QFontComboBox\"" -- 'src/*.ui' | grep --invert-match skip-keyword-check)

if [[  ${FOUND} ]]; then
  echo "Base QFontComboBox class used in .ui file!"
  echo " -> Use QgsFontComboBox class instead"
  echo
  echo "${FOUND}"
  echo
  RES=1
fi

FOUND=$(git grep "new QFontComboBox" -- 'src' | grep --invert-match skip-keyword-check)

if [[  ${FOUND} ]]; then
  echo "Base QFontComboBox class used in file!"
  echo " -> Use QgsFontComboBox class instead"
  echo "    or mark with // skip-keyword-check"  echo
  echo "${FOUND}"
  echo
  RES=1
fi


popd > /dev/null || exit

if [ $RES ]; then
  echo " *** Found QFontComboBox use in ui files"
  exit 1
fi

