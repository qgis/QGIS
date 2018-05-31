#!/usr/bin/env bash

# 

# Denis Rouzaud
# 13.01.2016

#set -e

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

FILEPATH=$1
OLDCLASS=$2
NEWCLASS=$3

OLDCLASSUPPER="${OLDCLASS^^}"
OLDCLASSLOWER="${OLDCLASS,,}"
NEWCLASSUPPER="${NEWCLASS^^}"
NEWCLASSLOWER="${NEWCLASS,,}"


if [[ ${FILEPATH:(-1)} = "/" ]]; then
  PATH="${FILEPATH::-1}"
fi

FILES=$(git ls-tree --name-only -r HEAD | grep ".*\.\(sip\|cpp\|h\|txt\)$")

for f in $FILES
do
	sed -i s/"$OLDCLASS"/"$NEWCLASS"/g "$f"
	sed -i s/"$OLDCLASSUPPER"/"$NEWCLASSUPPER"/g "$f"
	sed -i s/"$OLDCLASSLOWER"/"$NEWCLASSLOWER"/g "$f"
done

git mv $DIR/../src/$FILEPATH/$OLDCLASSLOWER.h   $DIR/../src/$FILEPATH/$NEWCLASSLOWER.h
git mv $DIR/../src/$FILEPATH/$OLDCLASSLOWER.cpp $DIR/../src/$FILEPATH/$NEWCLASSLOWER.cpp
git mv $DIR/../python/$FILEPATH/$OLDCLASSLOWER.sip $DIR/../python/$FILEPATH/$NEWCLASSLOWER.sip

