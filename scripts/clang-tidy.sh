#!/usr/bin/env bash
###########################################################################
#    clang-tidy.sh
#    ---------------------
#    Date                 : September 2022
#    Copyright            : (C) 2022 by Julien Cabieces
#    Email                : julien dot cabieces at oslandia dot com
###########################################################################
#                                                                         #
#   This program is free software; you can redistribute it and/or modify  #
#   it under the terms of the GNU General Public License as published by  #
#   the Free Software Foundation; either version 2 of the License, or     #
#   (at your option) any later version.                                   #
#                                                                         #
###########################################################################

set -e

usage() {
  echo 'Usage: '$(basename $0)' [-p <build_dir>] [-a] [-m <module_name>] [source_files]

       -a : run clang-tidy on all source files
       -m <module> : run clang-tidy on all module source files (core, gui, analysis...)

       Use either -a or -m, or source files' 1>&2; exit 1; }

SCRIPT_DIR=$(dirname $0)/..
SOURCE_DIR=$(realpath $SCRIPT_DIR)

BUILD_DIR=$(pwd)
while getopts "p:am:" o; do
    case "${o}" in
      p)
          BUILD_DIR=${OPTARG}
          ;;
      a)
          if [[ -n "$FILE_OPT" ]]; then
            usage
          fi
          FILE_OPT="ALL"
          ;;
        m)
          if [[ -n "$FILE_OPT" ]]; then
            usage
          fi
          FILE_OPT=${OPTARG}
          ;;
        *)
          usage
          ;;
    esac
done
shift $((OPTIND-1))

CLANG_TIDY_CHECKS=$(cat ${SOURCE_DIR}/tests/code_layout/clangtidy_checks.txt | grep -ve "^#" | grep -ve "^$" | tr -d '\n')

if [[ ! -f "$BUILD_DIR/compile_commands.json" ]]; then
  echo "compile_commands.json file is missing, you need to add -DCMAKE_EXPORT_COMPILE_COMMANDS=ON when you run cmake to generate it."
  exit 1
fi

if [[ -z "$FILE_OPT" ]]; then
  if [[ $# -lt 1 ]]; then
    echo "Missing files"
    usage
  else
    FILES=$*
  fi
else
  if [[ $# -gt 0 ]]; then
    usage
  elif [[ "$FILE_OPT" = "ALL" ]]; then
       FILES=$(find $SOURCE_DIR/src -name "*.cpp" -o -name "*.h")
  else
       FILES=$(find $SOURCE_DIR/src/$FILE_OPT -name "*.cpp" -o -name "*.h")
  fi
fi

set +e

for file in $FILES;
do
  clang-tidy -p=$BUILD_DIR -checks="$CLANG_TIDY_CHECKS" $file
done
