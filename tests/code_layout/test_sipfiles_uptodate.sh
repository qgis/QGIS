#!/usr/bin/env bash

DIR=$(git rev-parse --show-toplevel)

# GNU prefix command for mac os support (gsed, gsplit)
GP=
if [[ "$OSTYPE" =~ darwin* ]]; then
  GP=g
fi

pushd ${DIR} > /dev/null

code=0
while read -r sipfile; do
    header=$(${GP}sed -E 's/(.*)\.sip/src\/\1.h/' <<< $sipfile)
    if [ ! -f $header ]; then
      echo "*** Missing header: $header for sipfile $sipfile"
    else
      outdiff=$(./scripts/sipify.pl $header | diff python/$sipfile -)
      if [[ -n $outdiff ]]; then
        echo " *** SIP file not up to date: $sipfile"
        code=1
      fi
    fi
done < <(
${GP}sed -n -r 's/^%Include (.*\.sip)/core\/\1/p' python/core/core_auto.sip
${GP}sed -n -r 's/^%Include (.*\.sip)/gui\/\1/p' python/gui/gui_auto.sip
${GP}sed -n -r 's/^%Include (.*\.sip)/analysis\/\1/p' python/analysis/analysis_auto.sip
${GP}sed -n -r 's/^%Include (.*\.sip)/server\/\1/p' python/server/server_auto.sip
  )


popd > /dev/null

exit $code
