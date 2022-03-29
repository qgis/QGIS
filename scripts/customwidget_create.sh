#!/usr/bin/env bash

# This script automatically creates custom widget plugin for a given widget class name.
# Use customwidget_create.sh QgsColorButton to create QgsColorButtonPlugin files.
# It uses author name and email from git config.

# Denis Rouzaud
# 13.01.2016

set -e

# GNU prefix command for bsd/mac os support (gsed, gsplit)
GP=
if [[ "$OSTYPE" == *bsd* ]] || [[ "$OSTYPE" =~ darwin* ]]; then
  GP=g
fi

CLASSNAME=$1

TODAY=$(date '+%d.%m.%Y')
YEAR=$(date '+%Y')

AUTHOR=$(git config user.name)
EMAIL=$(git config user.email)

CLASSUPPER="${CLASSNAME^^}"
CLASSLOWER="${CLASSNAME,,}"
CLASSWITHOUTQGS=$(${GP}sed 's/^Qgs//' <<< ${CLASSNAME})

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

declare -a EXT=("cpp" "h")
for i in "${EXT[@]}"
do
	DESTFILE=$DIR/../src/customwidgets/${CLASSLOWER}plugin.$i
	cp "$DIR"/customwidget."$i".template "$DESTFILE"
	${GP}sed -i "s/%DATE%/${TODAY}/g" ${DESTFILE}
	${GP}sed -i "s/%YEAR%/${YEAR}/g" ${DESTFILE}
	${GP}sed -i "s/%AUTHOR%/${AUTHOR}/g" ${DESTFILE}
	${GP}sed -i "s/%EMAIL%/${EMAIL}/g" ${DESTFILE}
	${GP}sed -i "s/%CLASSUPPERCASE%/${CLASSUPPER}/g" ${DESTFILE}
	${GP}sed -i "s/%CLASSLOWERCASE%/${CLASSLOWER}/g" ${DESTFILE}
	${GP}sed -i "s/%CLASSMIXEDCASE%/${CLASSNAME}/g" ${DESTFILE}
	${GP}sed -i "s/%CLASSWITHOUTQGS%/${CLASSWITHOUTQGS}/g" ${DESTFILE}
done
