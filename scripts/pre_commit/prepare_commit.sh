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

set -e

if ! type -p "${TOPLEVEL}"/scripts/astyle.sh >/dev/null; then
  echo astyle.sh not found
  exit 1
fi

# capture files passed by pre-commit
MODIFIED="$@"

if [ -z "$MODIFIED" ]; then
  exit 0
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
  "${TOPLEVEL}"/scripts/pre_commit/sort_includes.py "$f"
  "${TOPLEVEL}"/scripts/pre_commit/doxygen_space.py "$f"

  # Run astyle only on src/core, others are handled by clang-format (see .pre-commit-config.yaml)
  if [[ $f =~ ^src/(core) ]]; then
    "${TOPLEVEL}"/scripts/astyle.sh "$f"
  fi
done

exit 0
