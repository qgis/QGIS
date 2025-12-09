#!/usr/bin/env bash
###########################################################################
#    prepare_commit.sh
#    ---------------------
#    Date                 : August 2008
#    Copyright            : (C) 2008 by Juergen E. Fischer
#    Email                : jef at norbit dot de
###########################################################################
#                                                                         #
#   This program is free software; you can redistribute it and/or modify  #
#   it under the terms of the GNU General Public License as published by  #
#   the Free Software Foundation; either version 2 of the License, or     #
#   (at your option) any later version.                                   #
#                                                                         #
###########################################################################

TOPLEVEL=$(git rev-parse --show-toplevel)

PATH=$TOPLEVEL/scripts:$PATH:$PWD/scripts

set -e

if ! type -p astyle.sh >/dev/null; then
  echo astyle.sh not found
  exit 1
fi

# capture files passed by pre-commit
MODIFIED="$@"

if [ -z "$MODIFIED" ]; then
  echo nothing was modified
  exit 0
fi

HAS_AG=false
if command -v ag > /dev/null; then
  HAS_AG=true
fi

HAS_UNBUFFER=false
if command -v unbuffer > /dev/null; then
  HAS_UNBUFFER=true
fi

# Run spell checker if requirements are met
if test "$HAS_AG" != "true"; then
  echo "WARNING: the ag(1) executable was not found, spell checker could not run" >&2
elif test "$HAS_UNBUFFER" != "true"; then
  echo "WARNING: the unbuffer(1) executable was not found, spell checker could not run" >&2
else
  "${TOPLEVEL}"/scripts/spell_check/check_spelling.sh "$MODIFIED"
fi

# Run doxygen layout test if requirements are met

MODIFIED_DOXYGEN_FILES=""
for f in $MODIFIED; do
  case "$f" in
    *.h|*.cpp)
      MODIFIED_DOXYGEN_FILES="$MODIFIED_DOXYGEN_FILES $f"
      ;;
  esac
done

if test "$HAS_AG" != "true"; then
  echo "WARNING: the ag(1) executable was not found, doxygen layout checker could not run" >&2
elif test "$HAS_UNBUFFER" != "true"; then
  echo "WARNING: the unbuffer(1) executable was not found, doxygen layout checker could not run" >&2
else
  "${TOPLEVEL}"/tests/code_layout/test_doxygen_layout.sh $MODIFIED_DOXYGEN_FILES
fi

MODIFIED_SHELLFILES=""
for f in $MODIFIED; do
  if [[ "$f" == *.sh ]]; then
    MODIFIED_SHELLFILES="$MODIFIED_SHELLFILES $f"
  fi
done

if [ -n "$MODIFIED_SHELLFILES" ]; then
  # Run shell checker if requirements are met
  if command -v shellcheck > /dev/null; then
    ${TOPLEVEL}/tests/code_layout/test_shellcheck.sh $MODIFIED_SHELLFILES
  else
    echo "WARNING: the shellcheck(1) executable was not found, shell checker could not run" >&2
  fi
fi

for f in $MODIFIED; do
  case "$f" in
  *.cpp|*.c|*.h|*.cxx|*.hxx|*.c++|*.h++|*.cc|*.hh|*.C|*.H|*.sip|*.mm)
    ;;
  *)
    continue
    ;;
  esac

  # Run Python formatters
  scripts/sort_includes.py "$f"
  scripts/doxygen_space.py "$f"

  # Run astyle only on src/core, others are handled by clang-format (see .pre-commit-config.yaml)
  if [[ $f =~ ^src/(core) ]]; then
    astyle.sh "$f"
  fi
done

exit 0
