#!/usr/bin/env bash

#set -e

DIR=$(git rev-parse --show-toplevel)

# GNU prefix command for mac os support (gsed, gsplit)
GP=
if [[ "$OSTYPE" =~ darwin* ]]; then
  GP=g
fi

pushd ${DIR} > /dev/null

modules=(core gui analysis server)

code=0
for module in "${modules[@]}"; do
  module_init=${DIR}/python/${module}/__init__.py
  cp ${module_init} ${DIR}/${module}.temp
  while read -r sipfile; do
      header=$(${GP}sed -E 's@(.*)\.sip@src/\1.h@; s@auto_generated/@@' <<< $sipfile)
      if [ ! -f $header ]; then
        echo "*** Missing header: $header for sipfile $sipfile"
      else
        outdiff=$(./scripts/sipify.pl -p ${DIR}/${module}.temp $header | diff python/$sipfile.in -)
        if [[ -n "$outdiff" ]]; then
          echo " *** SIP file not up to date: $sipfile"
          code=1
        fi
      fi
  done < <(
      ${GP}sed -n -r "s@^%Include auto_generated/(.*\.sip)@${module}/auto_generated/\1@p" python/${module}/${module}_auto.sip
  )
  outdiff=$(diff ${module_init} ${DIR}/${module}.temp)
  if [[ -n "$outdiff" ]]; then
    echo " *** Python ${module} (${module_init}) init not up to date: run sipify with -p argument"
    code=1
  fi
  rm ${DIR}/${module}.temp
done


popd > /dev/null

exit $code
