#!/usr/bin/env bash
###########################################################################
#    prepare_commit_spell_check.sh
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

exit 0
