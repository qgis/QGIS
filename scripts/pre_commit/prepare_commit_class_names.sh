#!/usr/bin/env bash

# This test checks for use of non-compliant class names

set -e

declare -a KEYWORDS=()
declare -a HINTS=()

KEYWORDS[0]="^\s*class[^:]*Qgs\S*3d"
HINTS[0]="Use '3D' capitalisation in class names instead of '3d'"

# capture files passed by pre-commit
MODIFIED="$@"

FILES_TO_CHECK=""
for f in $MODIFIED; do
  case "$f" in
    *.h|*.cpp)
      FILES_TO_CHECK="$FILES_TO_CHECK $f"
      ;;
  esac
done

if [ -z "$FILES_TO_CHECK" ]; then
  echo nothing was modified
  exit 0
fi

RES=

for i in "${!KEYWORDS[@]}"
do
  FOUND=$(grep -nH "${KEYWORDS[$i]}" $FILES_TO_CHECK 2>/dev/null | sed -n 's/.*\(Qgs\w*\).*/\1/p' | sort -u || true)

  if [[ ${FOUND} ]]; then
    echo "Found classes with non-standard names!"
    echo " -> ${HINTS[$i]}"
    echo
    echo "${FOUND}"
    echo
    RES=1
  fi

done

if [ $RES ]; then
  echo " *** Found non-compliant class names"
  exit 1
fi

