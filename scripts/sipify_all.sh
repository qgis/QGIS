#!/usr/bin/env bash
###########################################################################
#    sipify_all.sh
#    ---------------------
#    Date                 : 25.03.2017
#    Copyright            : (C) 2017 by Denis Rouzaud
#    Email                : denis.rouzaud@gmail.com
###########################################################################
#                                                                         #
#   This program is free software; you can redistribute it and/or modify  #
#   it under the terms of the GNU General Public License as published by  #
#   the Free Software Foundation; either version 2 of the License, or     #
#   (at your option) any later version.                                   #
#                                                                         #
###########################################################################
set -e

DIR=$(git rev-parse --show-toplevel)

# GNU prefix command for mac os support (gsed, gsplit)
GP=
if [[ "$OSTYPE" =~ darwin* ]]; then
  GP=g
fi

pushd ${DIR} > /dev/null

while read -r sipfile; do
  if ! grep -Fxq "$sipfile" python/auto_sip.blacklist; then
    echo "$sipfile"
    header=$(sed -E 's/(.*)\.sip/src\/\1.h/' <<< $sipfile)
    if [ ! -f $header ]; then
      echo "*** Missing header: $header for sipfile $sipfile"
    else
      ./scripts/sipify.pl $header > python/$sipfile
    fi
  fi
done < <(
sed -n -r 's/^%Include (.*\.sip)/core\/\1/p' python/core/core.sip
sed -n -r 's/^%Include (.*\.sip)/gui\/\1/p' python/gui/gui.sip
sed -n -r 's/^%Include (.*\.sip)/analysis\/\1/p' python/analysis/analysis.sip
  )

popd > /dev/null
