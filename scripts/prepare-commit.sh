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

if ! tty -s && [[ "$0" =~ /pre-commit ]]; then
    exec </dev/tty
fi

cd $TOPLEVEL

# GNU prefix command for mac os support (gsed, gsplit)
GP=
if [[ "$OSTYPE" =~ darwin* ]]; then
  GP=g
fi

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
MODIFIED=$(git status --porcelain| ${GP}sed -ne "s/^ *[MA]  *//p" | sort -u)

if [ -z "$MODIFIED" ]; then
  echo nothing was modified
  exit 0
fi

if [[ -n "$QGIS_CHECK_SPELLING" && -x ${TOPLEVEL}/scripts/spell_check/check_spelling.sh ]]; then ${TOPLEVEL}/scripts/spell_check/check_spelling.sh $MODIFIED; fi


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
  src/core/gps/qextserialport/*|src/plugins/globe/osgEarthQt/*|src/plugins/globe/osgEarthUtil/*|src/3d/poly2tri/*)
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
else
  rm $ASTYLEDIFF
fi


# verify SIP files
SIPIFYDIFF=sipify.$REV.diff
>$SIPIFYDIFF
for f in $MODIFIED; do
  # if cpp header
  if [[ $f =~ ^src\/(core|gui|analysis|server)\/.*\.h$ ]]; then
    # look if corresponding SIP file
    sip_include=$(${GP}sed -r 's/^src\/(\w+)\/.*$/python\/\1\/\1.sip/' <<< $f )
    sip_file=$(${GP}sed -r 's/^src\/(core|gui|analysis|server)\///; s/\.h$/.sip/' <<<$f )
    module=$(${GP}sed -r 's/^src\/(core|gui|analysis|server)\/.*$/\1/' <<<$f )
    if grep -Fq "$sip_file" ${TOPLEVEL}/python/${module}/${module}_auto.sip; then
      sip_file=$(${GP}sed -r 's/^src\///; s/\.h$/.sip/' <<<$f )
      m=python/$sip_file.$REV.prepare
      touch python/$sip_file
      cp python/$sip_file $m
      ${TOPLEVEL}/scripts/sipify.pl $f > python/$sip_file
      if ! diff -u $m python/$sip_file >>$SIPIFYDIFF; then
        echo "python/$sip_file is not up to date"
      fi
      rm $m
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
if [ -s "$ASTYLEDIFF" ]; then
    exit 1
fi

exit 0

# vim: set ts=2 expandtab :
