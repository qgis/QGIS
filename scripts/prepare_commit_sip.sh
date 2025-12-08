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

if ! tty -s && [[ "$0" =~ /pre-commit ]]; then
    exec </dev/tty
fi

# capture files passed by pre-commit
MODIFIED="$@"

if [ -z "$MODIFIED" ]; then
  echo nothing was modified
  exit 0
fi

REV=$(git log -n1 --pretty=%H)

# verify SIP files

for root_dir in python python/PyQt6; do

  if [[ $root_dir == "python/PyQt6" ]]; then
    IS_QT6="-qt6"
  fi

  for f in $MODIFIED; do
    # if cpp header
    if [[ $f =~ ^src\/(core|gui|analysis|server|3d)\/.*\.h$ ]]; then
      # look if corresponding SIP file
      sip_file=$(${GP}sed -r 's@^src/(core|gui|analysis|server|3d)/@@; s@\.h$@.sip@' <<<"$f" )
      pyfile=$(${GP}sed -E 's@([^\/]+\/)*([^\/]+)\.sip@\2.py@;' <<< "$sip_file")
      module=$(${GP}sed -r 's@src/(core|gui|analysis|server|3d)/.*$@\1@' <<<"$f" )
      if grep -Fq "$sip_file" "${TOPLEVEL}"/$root_dir/"${module}"/"${module}"_auto.sip; then
        sip_file=$(${GP}sed -r 's@^src/(core|gui|analysis|server|3d)@\1/auto_generated@; s@\.h$@.sip.in@' <<<"$f" )
        m=$root_dir/$sip_file.$REV.prepare
        if [ ! -f $root_dir/"$sip_file" ]; then
          touch $root_dir/"$sip_file"
        fi
        cp $root_dir/"$sip_file" "$m"
        "${TOPLEVEL}"/scripts/sipify.py $IS_QT6 -sip_output $m -python_output $root_dir/"${module}"/auto_additions/"${pyfile}" "$f"
        # only replace sip files if they have changed
        if ! cmp -s $root_dir/"$sip_file" "$m"; then
          echo "$root_dir/$sip_file is not up to date"
          cp "$m" $root_dir/"$sip_file"
        fi
        rm "$m"
      fi
    fi
  done
done

exit $FILES_CHANGED
