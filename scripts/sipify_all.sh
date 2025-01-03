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

CLASS_MAP=0
while getopts "m" opt; do
  case $opt in
  m)
    CLASS_MAP=1
    ;;
  \?)
    echo "Invalid option: -$OPTARG" >&2
    exit 1
    ;;
  esac
done
shift $((OPTIND-1))

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

pids=()
iPid=0

for root_dir in python python/PyQt6; do

  if [[ $root_dir == "python/PyQt6" ]]; then
    IS_QT6="-qt6"
  fi

  for module in "${modules[@]}"; do
    module_dir=${root_dir}/${module}

    if [[ ${CLASS_MAP} -eq 1 ]]; then
      rm ${module_dir}/class_map.yaml || true
      touch ${module_dir}/class_map.yaml
    fi

    # clean auto_additions and auto_generated folders
    rm -rf ${module_dir}/auto_additions/*.py
    rm -rf ${module_dir}/auto_generated/*.py
    # put back __init__.py
    echo '"""
This folder is completed using sipify.py script
It is not aimed to be manually edited
"""' > ${module_dir}/auto_additions/__init__.py

    while read -r sipfile; do
      echo "$sipfile.in"
      header=$(${GP}sed -E 's@(.*)\.sip@src/\1.h@; s@auto_generated/@@' <<< $sipfile)
      pyfile=$(${GP}sed -E 's@([^\/]+\/)*([^\/]+)\.sip@\2.py@;' <<< $sipfile)
      if [ ! -f $header ]; then
        echo "*** Missing header: $header for sipfile $sipfile"
      else
        path=$(${GP}sed -r 's@/[^/]+$@@' <<< $sipfile)
        mkdir -p python/$path
        CLASS_MAP_CALL=
        if [[ ${CLASS_MAP} -eq 1 ]]; then
          CLASS_MAP_CALL="-c ${module_dir}/class_map.yaml"
        fi
        ./scripts/sipify.py $IS_QT6 -s ${root_dir}/${sipfile}.in -p ${module_dir}/auto_additions/${pyfile} ${CLASS_MAP_CALL} ${header} &
        pids[iPid]=$!
        iPid=$((iPid+1))

      fi
      count=$((count+1))
    done < <( ${GP}sed -n -r "s@^%Include auto_generated/(.*\.sip)@${module}/auto_generated/\1@p" python/${module}/${module}_auto.sip )
  done
done

for pid in "${pids[@]}"; do
    wait $pid || ( echo "Errors while calling sipify!!!" && exit 1 )
done

if [[ ${CLASS_MAP} -eq 1 ]]; then
  for root_dir in python python/PyQt6; do
    for module in "${modules[@]}"; do
      module_dir=${root_dir}/${module}
      echo "sorting ${module_dir}/class_map.yaml"
      sort -n -o ${module_dir}/class_map.yaml ${module_dir}/class_map.yaml
    done
  done
fi

echo " => $count files sipified! 🍺"

popd > /dev/null
