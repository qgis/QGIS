#!/bin/bash

# This script automatically creates custom widget plugin for a given widget class name.
# Use customwidget_create.sh QgsColorButton to create QgsColorButtonPlugin files.
# It uses author name and email from git config.

# Denis Rouzaud
# 13.01.2016

set -e

CLASSNAME=$1

TODAY=`date '+%d.%m.%Y'`
YEAR=`date '+%Y'`

AUTHOR=`git config user.name`
EMAIL=`git config user.email`

CLASSUPPER="${CLASSNAME^^}"
CLASSLOWER="${CLASSNAME,,}"

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

declare -a EXT=("cpp" "h")
for i in "${EXT[@]}"
do
	DESTFILE=$DIR/../src/customwidgets/${CLASSLOWER}plugin.$i
	cp $DIR/customwidget_template.$i $DESTFILE
	sed -i s/%DATE%/"$TODAY"/g "$DESTFILE"
	sed -i s/%YEAR%/"$YEAR"/g "$DESTFILE"
	sed -i s/%AUTHOR%/"$AUTHOR"/g "$DESTFILE"
	sed -i s/%EMAIL%/"$EMAIL"/g "$DESTFILE"
	sed -i s/%CLASSUPPERCASE%/"$CLASSUPPER"/g "$DESTFILE"
	sed -i s/%CLASSLOWERCASE%/"$CLASSLOWER"/g "$DESTFILE"
	sed -i s/%CLASSMIXEDCASE%/"$CLASSNAME"/g "$DESTFILE"
done


