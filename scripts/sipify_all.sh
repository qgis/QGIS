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

# TEMPLATE_DOC=""
# while :; do
#     case $1 in
#         -t|--template-doc) TEMPLATE_DOC="-template-doc"
#         ;;
#         *) break
#     esac
#     shift
# done

DIR=$(git rev-parse --show-toplevel)

# GNU prefix command for bsd/mac os support (gsed, gsplit)
GP=
if [[ "$OSTYPE" == *bsd* ]] || [[ "$OSTYPE" =~ darwin* ]]; then
  GP=g
fi

pushd ${DIR} > /dev/null

count=0

if [[ -n $1 ]]; then
  modules=("$1")
else
  modules=(core gui analysis server 3d)
fi
for module in "${modules[@]}"; do

  # clean auto_additions and auto_generated folders
  rm -rf python/${module}/auto_additions/*.py
  rm -rf python/${module}/auto_generated/*.py
  # put back __init__.py
  echo '"""
This folder is completed using sipify.pl script
It is not aimed to be manually edited
"""' > python/${module}/auto_additions/__init__.py

  while read -r sipfile; do
      echo "$sipfile.in"
      header=$(${GP}sed -E 's@(.*)\.sip@src/\1.h@; s@auto_generated/@@' <<< $sipfile)
      pyfile=$(${GP}sed -E 's@([^\/]+\/)*([^\/]+)\.sip@\2.py@;' <<< $sipfile)
      if [ ! -f $header ]; then
        echo "*** Missing header: $header for sipfile $sipfile"
      else
        path=$(${GP}sed -r 's@/[^/]+$@@' <<< $sipfile)
        mkdir -p python/$path
        ./scripts/sipify.pl -s python/$sipfile.in -p python/${module}/auto_additions/${pyfile} $header &
      fi
      count=$((count+1))
  done < <( ${GP}sed -n -r "s@^%Include auto_generated/(.*\.sip)@${module}/auto_generated/\1@p" python/${module}/${module}_auto.sip )
done
wait # wait for sipify processes to finish

echo " => $count files sipified! 🍺"

popd > /dev/null
