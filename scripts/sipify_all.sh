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

count=0

modules=(core gui analysis server)
for module in "${modules[@]}"; do
  while read -r sipfile; do
      echo "$sipfile"
      header=$(${GP}sed -E 's/(.*)\.sip/src\/\1.h/' <<< $sipfile)
      if [ ! -f $header ]; then
        echo "*** Missing header: $header for sipfile $sipfile"
      else
        path=$(${GP}sed -r 's@/[^/]+$@@' <<< $sipfile)
        mkdir -p python/$path
        ./scripts/sipify.pl $header > python/$sipfile
      fi
      count=$((count+1))
  done < <( ${GP}sed -n -r "s/^%Include (.*\.sip)/${module}\/\1/p" python/${module}/${module}_auto.sip )
done

echo " => $count files sipified! 🍺"

popd > /dev/null
