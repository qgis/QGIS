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

if ! tty -s && [[ "$0" =~ /pre-commit ]]; then
    exec </dev/tty
fi

cd "$TOPLEVEL"

# GNU prefix command for bsd/mac os support (gsed, gsplit)
GP=
if [[ "$OSTYPE" == *bsd* ]] || [[ "$OSTYPE" =~ darwin* ]]; then
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

# Run doxygen layout test if requirements are met
if test "$HAS_AG" != "true"; then
  echo "WARNING: the ag(1) executable was not found, doxygen layout checker could not run" >&2
elif test "$HAS_UNBUFFER" != "true"; then
  echo "WARNING: the unbuffer(1) executable was not found, doxygen layout checker could not run" >&2
else
  "${TOPLEVEL}"/tests/code_layout/test_doxygen_layout.sh $MODIFIED
fi

MODIFIED_SHELLFILES=$(echo "${MODIFIED}" | grep '\.sh$' || true)
if [ -n "$MODIFIED_SHELLFILES" ]; then
  # Run shell checker if requirements are met
  if command -v shellcheck > /dev/null; then
    ${TOPLEVEL}/tests/code_layout/test_shellcheck.sh "${MODIFIED_SHELLFILES}"
  else
    echo "WARNING: the shellcheck(1) executable was not found, shell checker could not run" >&2
  fi
fi

FILES_CHANGED=0

# save original changes
REV=$(git log -n1 --pretty=%H)
#git diff >sha-"$REV".diff

ASTYLEDIFF=astyle.$REV.diff
true > "$ASTYLEDIFF"

# reformat
i=0
N=$(echo "$MODIFIED" | wc -w)
for f in $MODIFIED; do
  (( i++ )) || true

  case "$f" in
  *.cpp|*.c|*.h|*.cxx|*.hxx|*.c++|*.h++|*.cc|*.hh|*.C|*.H|*.sip|*.mm)
    ;;

  *)
    continue
    ;;
  esac

  # only run astyle on sipified directories, others are handled by clang-format (see .pre-commit-config.yaml)
	if [[ $f =~ ^src/(core) ]]; then
    m=$f.$REV.prepare
    cp "$f" "$m"
    ASTYLEPROGRESS=" [$i/$N]" astyle.sh "$f"
    if diff -u "$m" "$f" >>"$ASTYLEDIFF"; then
      # no difference found
      rm "$m"
    fi
  fi
done

if [ -s "$ASTYLEDIFF" ]; then
  if tty -s; then
    # review astyle changes
    colordiff <"$ASTYLEDIFF" | less -r
    rm "$ASTYLEDIFF"
  else
    echo "Files changed (see $ASTYLEDIFF)"
  fi
  FILES_CHANGED=1
else
  rm "$ASTYLEDIFF"
fi


# verify SIP files
SIPIFYDIFF=sipify.$REV.diff
true > "$SIPIFYDIFF"

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
        if ! diff -u $root_dir/"$sip_file" "$m" >>"$SIPIFYDIFF"; then
          echo "$root_dir/$sip_file is not up to date"
          cp "$m" $root_dir/"$sip_file"
        fi
        rm "$m"
      fi
    fi
  done
done
if [[ -s "$SIPIFYDIFF" ]]; then
  if tty -s; then
    # review astyle changes
    colordiff <"$SIPIFYDIFF" | less -r
    rm "$SIPIFYDIFF"
  else
    echo "Files changed (see $ASTYLEDIFF)"
  fi
  FILES_CHANGED=1
else
  rm "$SIPIFYDIFF"
fi

exit $FILES_CHANGED

# vim: set ts=2 expandtab :
