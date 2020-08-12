#!/usr/bin/env bash

#set -e

srcdir=$(dirname $0)/../../

DIR=$(git -C ${srcdir} rev-parse --show-toplevel)

# GNU prefix command for mac os support (gsed, gsplit)
GP=
if [[ "$OSTYPE" =~ darwin* ]]; then
  GP=g
fi

pushd ${DIR} > /dev/null || exit

modules=(core gui analysis server)

code=0
for module in "${modules[@]}"; do
  while read -r sipfile; do
      header=$(${GP}sed -E 's@(.*)\.sip@src/\1.h@; s@auto_generated/@@' <<< $sipfile)
      pyfile=$(${GP}sed -E 's@([^\/]+\/)*([^\/]+)\.sip@\2.py@;' <<< $sipfile)
      if [ ! -f $header ]; then
        echo "*** Missing header: $header for sipfile $sipfile"
      else
        outdiff=$(./scripts/sipify.pl -p python/${module}/auto_additions/${pyfile}.temp $header | diff python/$sipfile.in -)
        if [[ -n "$outdiff" ]]; then
          echo " *** SIP file not up to date: $sipfile"
          echo " $outdiff "
          code=1
        fi
        if [[ -f python/${module}/auto_additions/${pyfile}.temp ]]; then
          outdiff2=$(diff python/${module}/auto_additions/${pyfile} python/${module}/auto_additions/${pyfile}.temp)
          if [[ -n "$outdiff2" ]]; then
            echo " *** Python addition file not up to date: $sipfile"
            echo " $outdiff2 "
            code=1
          fi
        fi
      fi
  done < <(
      ${GP}sed -n -r "s@^%Include auto_generated/(.*\.sip)@${module}/auto_generated/\1@p" python/${module}/${module}_auto.sip
  )
done


popd > /dev/null || exit

exit $code
