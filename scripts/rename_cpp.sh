#!/usr/bin/env bash

# This scripts renames the name of a class as well as its header and cpp file
# (assuming they are the lowercase version of the class name).
# 
# Usage: ./scripts/rename_cpp.sh src/core QgsMyClassName QgsMyNewClassName

set -e

# GNU prefix command for bsd/mac os support (gsed, gsplit)
GP=
if [[ "$OSTYPE" == *bsd* ]] || [[ "$OSTYPE" =~ darwin* ]]; then
  GP=g
fi

FILEPATH=$1
OLD_CLASSNAME=$2
NEW_CLASSNAME=$3

OLD_CLASSUPPER="${OLD_CLASSNAME^^}"
OLD_CLASSLOWER="${OLD_CLASSNAME,,}"
NEW_CLASSUPPER="${NEW_CLASSNAME^^}"
NEW_CLASSLOWER="${NEW_CLASSNAME,,}"


FILES=$(ag -c $OLD_CLASSNAME | cut -d: -f1)

for f in ${FILES}; do
  ${GP}sed -i s/${OLD_CLASSNAME}/${NEW_CLASSNAME}/g $f
  ${GP}sed -i s/${OLD_CLASSUPPER}/${NEW_CLASSUPPER}/g $f
  ${GP}sed -i s/${OLD_CLASSLOWER}/${NEW_CLASSLOWER}/g $f
done

set +e

mv ${FILEPATH}/${OLD_CLASSLOWER}.h ${FILEPATH}/${NEW_CLASSLOWER}.h
mv ${FILEPATH}/${OLD_CLASSLOWER}.cpp ${FILEPATH}/${NEW_CLASSLOWER}.cpp
