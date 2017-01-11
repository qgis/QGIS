#!/bin/bash
###########################################################################
#    chkspelling_ag.sh
#    ---------------------
#    Date                 : December 2016
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

# optional arguments: files to be checked
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

AGIGNORE=${DIR}/.agignore

# This will try to look for mispelling within larger words.
# Condition is hard to explain in words.
# You can test it here: https://regex101.com/r/7kznVA/9
# extra words that should not be checked in longer words
WHOLEWORDS=$(echo "("; perl -ne 'print if not /^(\w)(\w)\w{2,}(\w)(\w):(\2\2|\1(?:(?!\1)\w)|(?:(?!\1)\w)\2|(?:(?!\1)\w)(?:(?!\1)\w)|\2\1)\w*(\3\3|(?:(?!\4)\w)(?:(?!\3)\4)|\3(?:(?!\4).)|(?:(?!\4)\w)(?:(?!\4)\w)|\4\3)(?!:\*)$/' scripts/spelling.dat | cut -d: -f1 |  tr '\n' '\|' | sed -e 's/|$//'; echo ")")
INWORDS=$(   echo "("; perl -ne 'print if     /^(\w)(\w)\w{2,}(\w)(\w):(\2\2|\1(?:(?!\1)\w)|(?:(?!\1)\w)\2|(?:(?!\1)\w)(?:(?!\1)\w)|\2\1)\w*(\3\3|(?:(?!\4)\w)(?:(?!\3)\4)|\3(?:(?!\4).)|(?:(?!\4)\w)(?:(?!\4)\w)|\4\3)(?!:\*)$/' scripts/spelling.dat | cut -d: -f1 |  tr '\n' '\|' | sed -e 's/|$//'; echo ")")

if [ ! $# -eq 0 ]; then
  EXCLUDE=$(cat $AGIGNORE | sed -e 's/\s*#.*$//' -e '/^\s*$/d' | tr '\n' '|' | sed -e 's/|$//')
  FILES=$(echo $@ | tr -s '[[:blank:]]' '\n' | egrep -iv "$EXCLUDE" | tr '\n' ' ' )
  echo "Running spell check on files: $FILES"
else
  FILES="."
fi

SPELLOK='(#\s*spellok|<!--#\s*spellok-->)$'


exec 5>&1
# "path-to-ignore" option differs on ag version: --path-to-ignore on fedora, --path-to-agignore on ubuntu 16.04: using short option
OUTPUT=$(unbuffer ag --smart-case --all-text --nopager --numbers --word-regexp -p $AGIGNORE "${WHOLEWORDS}"'(?!.*'"${SPELLOK}"')' $FILES | tee /dev/fd/5 ; \
         unbuffer ag --smart-case --all-text --nopager --numbers               -p $AGIGNORE "${INWORDS}"'(?!.*'"${SPELLOK}"')'    $FILES | tee /dev/fd/5)


ESCSPELLOK=$(echo $SPELLOK | sed 's/(/\\\\(/' | sed 's/)/\\\\)/' | sed 's/|/\\\\|/')

if [[ !  -z  $OUTPUT  ]]; then
  echo "Spelling errors have been found"
  echo "****"
  # < ---------- get files + error --------------------------------------------------------------------------->                                                                     <-- generate sed command ....                          <------------------------------ get correction word ---------------------------------->    <------------------------------- match case ------------------------------------------->   <-----replace : by / and add word boundary------> ...finalize sed command> remove duplicate line
  ag --smart-case --only-matching --nogroup --nonumbers --all-text --word-regexp -p $AGIGNORE "${WHOLEWORDS}"'(?!.*'"${SPELLOK}"')' $FILES | sed -e 's/\(\S*\):\([[:alnum:]]*\)$/  echo "sed -i \x27\/'"$ESCSPELLOK"'\/! s\/"$( echo "\2:$(ag --nonumbers --ignore-case --word-regexp \2 scripts\/spelling.dat | cut -d: -f2)" | sed -r \x27s\/([A-Z]+):(.*)\/\\1:\\U\\2\/; s\/([A-Z][a-z]+):([a-z])\/\\1:\\U\\2\\L\/\x27  | sed -r \x27s\/(\\S\*):\/\\\\b\\1\\\\b\\\/\/\x27)"\/g\x27 \1" /e'  | sort -u
  ag --smart-case --only-matching --nogroup --nonumbers --all-text               -p $AGIGNORE "${INWORDS}"'(?!.*'"${SPELLOK}"')'    $FILES | sed -e 's/\(\S*\):\([[:alnum:]]*\)$/  echo "sed -i \x27\/'"$ESCSPELLOK"'\/! s\/"$( echo "\2:$(ag --nonumbers --ignore-case --word-regexp \2 scripts\/spelling.dat | cut -d: -f2)" | sed -r \x27s\/([A-Z]+):(.*)\/\\1:\\U\\2\/; s\/([A-Z][a-z]+):([a-z])\/\\1:\\U\\2\\L\/\x27  | sed -r \x27s\/(\\S\*):\/\\1\\\/\/\x27)"\/g\x27 \1" /e'            | sort -u
  echo "****"
  echo "Run above commands to fix spelling errors or add #spellok at the end of the line to discard spell check on this line."
  exit 1
else
  exit 0
fi
