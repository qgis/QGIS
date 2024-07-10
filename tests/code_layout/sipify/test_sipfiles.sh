#!/usr/bin/env bash

#set -e

srcdir=$(dirname $0)/../../

DIR=$(git -C ${srcdir} rev-parse --show-toplevel)

# GNU prefix command for bsd/mac os support (gsed, gsplit)
GP=
if [[ "$OSTYPE" == *bsd* ]] || [[ "$OSTYPE" =~ darwin* ]]; then
  GP=g
fi

pushd ${DIR} > /dev/null || exit

modules=(3d core gui analysis server)

code=0
for root_dir in python python/PyQt6; do

  if [[ $root_dir == "python/PyQt6" ]]; then
    IS_QT6="--qt6"
  fi

  for module in "${modules[@]}"; do
    while read -r sipfile; do
      header=$(${GP}sed -E 's@(.*)\.sip@src/\1.h@; s@auto_generated/@@' <<< $sipfile)
      pyfile=$(${GP}sed -E 's@([^\/]+\/)*([^\/]+)\.sip@\2.py@;' <<< $sipfile)
      if [ ! -f $header ]; then
        echo "*** Missing header: $header for sipfile $sipfile"
      else
        outdiff=$(./scripts/sipify.pl $IS_QT6 -p $root_dir/${module}/auto_additions/${pyfile}.temp -c $root_dir/${module}/class_map.yaml.test $header | diff $root_dir/$sipfile.in -)
        if [[ -n "$outdiff" ]]; then
          echo " *** SIP file not up to date: $root_dir/$sipfile"
          echo " $outdiff "
          code=1
        fi
        if [[ -f $root_dir/${module}/auto_additions/${pyfile}.temp ]]; then
          outdiff2=$(diff $root_dir/${module}/auto_additions/${pyfile} $root_dir/${module}/auto_additions/${pyfile}.temp)
          if [[ -n "$outdiff2" ]]; then
            echo " *** Python addition file not up to date: $root_dir/$sipfile"
            echo " $outdiff2 "
            code=1
          fi
        fi
      fi
    done < <(
      ${GP}sed -n -r "s@^%Include auto_generated/(.*\.sip)@${module}/auto_generated/\1@p" $root_dir/${module}/${module}_auto.sip
    )
    sort -n -o ${module_dir}/class_map.dat.test ${module_dir}/class_map.dat.test
    outdiff3=$($root_dir/${module}/class_map.yaml $root_dir/${module}/class_map.yaml.test)
    if [[ -n "$outdiff3" ]]; then
      echo " *** Class map not up to date: $root_dir/${module}/class_map.yaml. Run sipify_all.sh <module> to fix this."
      echo " $outdiff3 "
      code=1
    fi
  done
done

popd > /dev/null || exit

exit $code
