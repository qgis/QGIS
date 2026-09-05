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

set -e

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
WORK=$(mktemp -d)
trap 'rm -rf "$WORK"' EXIT
cd "$WORK"

cat > spelling_error.txt <<'EOF'
As you would'nt
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
"MyErrror"
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
EOF

cat > expected <<'EOF'
1 would'nt wouldn't
2 Are'nt aren't
3 should'nt shouldn't
4 australia Australia
5 abouta about a
6 abouta about a
6 wont won't
7 abbout about
8 abotu about
9 abov above
10 Extint Extinct
11 Feeded Fed
12 EXLUSIVE EXCLUSIVE
13 exept except
14 ABSOLUT ABSOLUTE
15 CRITERIAS CRITERIA
16 Criterias Criteria
17 ABSOLUT ABSOLUTE
18 Errror Error
19 Errupted Erupted
20 everytime every time
21 Aditional Additional
22 Symbo Symbol
23 continous continuous
EOF

cat > fixed <<'EOF'
As you wouldn't
aren't you dumb?
You shouldn't be there
welcome to Australia
it's all about a cat
about a thse two errors on the same line you won't know anything
allaboutme
about a dog
put that above my head
MyExtinctIsNotHereYet
FedCats
EXCLUSIVE is upper case
_except has underscore
_ABSOLUTE_ has too
CRITERIA_
_Criteria
_ABSOLUTE
"MyError"
VolcanoErupted
every time I get drunk
TrAdditional is not traditional
graduatedSymbol)
(continuous)
# !!! NO ERROR UNDER THIS LINE !!!
aboutarabbit
abovyour shoulder
there is no errror # spellok
it is ABSOLUTE)
_ABSOLUTE_
EOF

STATUS=0

# the errors are reported, at the right line, with the right correction
"${DIR}"/check_spelling.py -l found.log spelling_error.txt > /dev/null || true
cut -d' ' -f2- found.log | LC_ALL=C sort > found
LC_ALL=C sort expected > want
if ! diff -u want found; then
  echo "SPELLING TEST FAILED: unexpected errors reported" >&2
  STATUS=1
fi

# --fix corrects them all, and leaves the rest of the file alone
"${DIR}"/check_spelling.py --fix spelling_error.txt > /dev/null || true
if ! diff -u fixed spelling_error.txt; then
  echo "SPELLING TEST FAILED: --fix did not produce the expected file" >&2
  STATUS=1
fi
if ! "${DIR}"/check_spelling.py spelling_error.txt; then
  echo "SPELLING TEST FAILED: errors remain after --fix" >&2
  STATUS=1
fi

if [[ $STATUS -eq 0 ]]; then
  echo "TEST OK"
fi
exit $STATUS
