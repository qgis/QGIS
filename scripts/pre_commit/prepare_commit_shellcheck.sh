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
set -e

# capture files passed by pre-commit
MODIFIED="$@"

if [ -z "$MODIFIED" ]; then
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

  echo "Running shell check on files: $MODIFIED_SHELLFILES"

  if command -v shellcheck > /dev/null; then
    pushd ${TOPLEVEL} > /dev/null || exit
    result=$(shellcheck -e SC2016,SC2015,SC2086,SC2002,SC1117,SC2154,SC2076,SC2046,SC1090,SC2038,SC2031,SC2030,SC2162,SC2044,SC2119,SC1001,SC2120,SC2059,SC2128,SC2005,SC2013,SC2027,SC2090,SC2089,SC2124,SC2001,SC2010,SC1072,SC1073,SC1009,SC2166,SC2045,SC2028,SC1091,SC1083,SC2021 ${MODIFIED_SHELLFILES} || true)
    popd > /dev/null || return

    if [[ $result ]]; then
      echo " *** shellcheck found script errors"
      echo "$result"
      exit 1
    fi
  else
    echo "WARNING: the shellcheck(1) executable was not found, shell checker could not run" >&2
  fi
fi

exit 0
