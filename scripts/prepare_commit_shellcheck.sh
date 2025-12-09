#!/usr/bin/env bash
###########################################################################
#    prepare_commit_shellcheck.sh
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

# capture files passed by pre-commit
MODIFIED="$@"

if [ -z "$MODIFIED" ]; then
  echo nothing was modified
  exit 0
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

exit 0
