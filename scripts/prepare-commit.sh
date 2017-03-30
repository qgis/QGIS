#!/usr/bin/env bash
###########################################################################
#    prepare-commit.sh
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

PATH=$TOPLEVEL/scripts:$PATH

cd $TOPLEVEL

if ! type -p astyle.sh >/dev/null; then
  echo astyle.sh not found
  exit 1
fi

if ! type -p colordiff >/dev/null; then
  colordiff()
  {
    cat "$@"
  }
fi

if [ "$1" = "-c" ]; then
  echo "Cleaning..."
  remove_temporary_files.sh
fi

set -e

# determine changed files
MODIFIED=$(git status --porcelain| sed -ne "s/^ *[MA]  *//p" | sort -u)

if [ -z "$MODIFIED" ]; then
  echo nothing was modified
  exit 0
fi

${TOPLEVEL}/scripts/spell_check/check_spelling.sh $MODIFIED

# save original changes
REV=$(git log -n1 --pretty=%H)
git diff >sha-$REV.diff

ASTYLEDIFF=astyle.$REV.diff
>$ASTYLEDIFF

# reformat
i=0
N=$(echo $MODIFIED | wc -w)
for f in $MODIFIED; do
  (( i++ )) || true

  case "$f" in
  src/core/gps/qextserialport/*|src/plugins/globe/osgEarthQt/*|src/plugins/globe/osgEarthUtil/*)
    echo $f skipped
    continue
    ;;

  *.cpp|*.c|*.h|*.cxx|*.hxx|*.c++|*.h++|*.cc|*.hh|*.C|*.H|*.sip|*.py)
    ;;

  *)
    continue
    ;;
  esac

  m=$f.$REV.prepare

  cp $f $m
  ASTYLEPROGRESS=" [$i/$N]" astyle.sh $f
  if diff -u $m $f >>$ASTYLEDIFF; then
    # no difference found
    rm $m
  fi
done

if [ -s "$ASTYLEDIFF" ]; then
  if tty -s; then
    # review astyle changes
    colordiff <$ASTYLEDIFF | less -r
  else
    echo "Files changed (see $ASTYLEDIFF)"
  fi
  exit 1
else
  rm $ASTYLEDIFF
fi



# verify SIP files
SIPIFYDIFF=sipify.$REV.diff
>$SIPIFYDIFF
for f in $MODIFIED; do
  # if cpp header
  if [[ $f =~ ^src\/(core|gui|analysis)\/.*\.h$ ]]; then
    # look if corresponding SIP file
    #echo $f
    sip_include=$(sed -r 's/^src\/(\w+)\/.*$/python\/\1\/\1.sip/' <<< $f )
    sip_file=$(sed -r 's/^src\/(core|gui|analysis)\///; s/\.h$/.sip/' <<<$f )
    if grep -Exq "^\s*%Include $sip_file" ${TOPLEVEL}/$sip_include ; then
      #echo "in SIP"
      sip_file=$(sed -r 's/^src\///; s/\.h$/.sip/' <<<$f )
      # check it is not blacklisted (i.e. manualy SIP)
      if ! grep -Fxq "$sip_file" python/auto_sip.blacklist; then
        #echo "automatic file"
        m=python/$sip_file.$REV.prepare
        touch python/$sip_file
        cp python/$sip_file $m
        ${TOPLEVEL}/scripts/sipify.pl $f > $m
        if diff -u $m python/$sip_file >>$SIPIFYDIFF; then
          # no difference found
          rm $m
        else
          echo "python/$sip_file is not up to date"
        fi
      fi
    fi
  fi
done
if [[ -s "$SIPIFYDIFF" ]]; then
  if tty -s; then
    # review astyle changes
    colordiff <$SIPIFYDIFF | less -r
  else
    echo "Files changed (see $ASTYLEDIFF)"
  fi
  exit 1
else
  rm $SIPIFYDIFF
fi


exit 0

# vim: set ts=8 noexpandtab :
