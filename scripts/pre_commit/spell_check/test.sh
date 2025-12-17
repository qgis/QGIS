#!/usr/bin/env bash
###########################################################################
#    test.sh
#    ---------------------
#    Date                 : January 2017
#    Copyright            : (C) 2016 by Denis Rouzaud
#    Email                : denis.rouzaud@gmail.com
###########################################################################
#                                                                         #
#   This program is free software; you can redistribute it and/or modify  #
#   it under the terms of the GNU General Public License as published by  #
#   the Free Software Foundation; either version 2 of the License, or     #
#   (at your option) any later version.                                   #
#                                                                         #
###########################################################################

# Testing the spell test :)

# GNU prefix command for bsd/mac os support (gsed, gsplit)
GP=
if [[ "$OSTYPE" == *bsd* ]] || [[ "$OSTYPE" =~ darwin* ]]; then
  GP=g
fi

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

echo "As you would'nt
Are'nt you dumb?
You should'nt be there
welcome to australia
it's all abouta cat
abouta thse two errors on the same line you wont know anything
allabboutme
abotu a dog
put that abov my head
MyExtintIsNotHereYet
FeededCats
EXLUSIVE is upper case
_exept has underscore
_ABSOLUT_ has too
CRITERIAS_
_Criterias
_ABSOLUT
\"MyErrror\"
VolcanoErrupted
everytime I get drunk
TrAditional is not traditional
graduatedSymbo)
(continous)
# !!! NO ERROR UNDER THIS LINE !!!
aboutarabbit
abovyour shoulder
there is no errror # spellok
it is ABSOLUTE)
_ABSOLUTE_
" > spelling_error.dat~

echo "spelling_error.dat~  1 would'nt wouldn't
spelling_error.dat~  2 Are'nt aren't
spelling_error.dat~  3 should'nt shouldn't
spelling_error.dat~  4 australia Australia
spelling_error.dat~  5 abouta about a
spelling_error.dat~  6 abouta about a
spelling_error.dat~  6 wont won't
spelling_error.dat~  7 abbout about
spelling_error.dat~  8 abotu about
spelling_error.dat~  9 abov above
spelling_error.dat~  10 Extint Extinct
spelling_error.dat~  11 Feeded Fed
spelling_error.dat~  12 EXLUSIVE EXCLUSIVE
spelling_error.dat~  13 exept except
spelling_error.dat~  14 ABSOLUT ABSOLUTE
spelling_error.dat~  15 CRITERIAS CRITERIA
spelling_error.dat~  16 Criterias Criteria
spelling_error.dat~  17 ABSOLUT ABSOLUTE
spelling_error.dat~  18 Errror Error
spelling_error.dat~  19 Errupted Erupted
spelling_error.dat~  20 everytime every time
spelling_error.dat~  21 Aditional Additional
spelling_error.dat~  22 Symbo Symbol
spelling_error.dat~  23 continous continuous" | ${GP}sort -u > spelling_error.expected~

rm -f spelling_error.log~
${DIR}/check_spelling.sh -r -l spelling_error.log~ spelling_error.dat~
${GP}sort -u -o spelling_error.log~ spelling_error.log~
DIFF=$(diff spelling_error.log~ spelling_error.expected~)

if [[ -n $DIFF ]]; then
  echo "SPELLING TEST FAILED" >&2
else
  echo "TEST OK"
fi
