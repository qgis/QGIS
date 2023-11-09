#!/usr/bin/env bash

if [ "$#" -ne 1 ] ; then
  echo "untwine_to_qgis: untwine directory argument required"
  exit 1
fi

EXTERNAL_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
UNTWINE_QGIS_DIR=$EXTERNAL_DIR/untwine

UNTWINE_DIR=$1
if [ ! -d "$UNTWINE_DIR/untwine" ] ; then
  echo "untwine_to_qgis: Directory $UNTWINE_DIR/untwine does not exist"
  exit 1
fi

PWD=`pwd`

echo "untwine_to_qgis: Remove old version"
rm -rf $UNTWINE_QGIS_DIR/*

echo "untwine_to_qgis: Copy new version"
rsync -r $UNTWINE_DIR/ $UNTWINE_QGIS_DIR/ --exclude="CMakeLists.txt*" --exclude="cmake/" --exclude="README.md" --exclude=".git" --exclude=".gitignore" --exclude=".github/" --exclude="ci/" --exclude="lazperf/"

echo "untwine_to_qgis: Done"
cd $PWD
