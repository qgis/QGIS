#!/bin/bash
###########################################################################
#    chkspelling.sh
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

RE=$(cut -d: -f1 scripts/spelling.dat |  tr '\n' '\|' | sed -e 's/|$//')
if [ ! $# -eq 0 ]; then
  EXCLUDE=$(cat $AGIGNORE | sed -e 's/\s*#.*$//' -e '/^\s*$/d' | tr '\n' '|' | sed -e 's/|$//')
  FILES=$(echo $@ | tr -s '[[:blank:]]' '\n' | egrep -iv "$EXCLUDE" | tr '\n' ' ' )
  echo "Running spell check on files: $FILES"
else
  FILES="."
fi


exec 5>&1
# "path-to-ignore" option differs on ag version: --path-to-ignore on fedora, --path-to-agignore on ubuntu 16.04: using short option
OUTPUT=$(unbuffer ag --smart-case --all-text --nopager --numbers --word-regexp -p $AGIGNORE "$RE" $FILES |tee /dev/fd/5)


if [[ !  -z  $OUTPUT  ]]; then
  echo "Spelling errors have been found"
  echo "****"
  # < ---------- get files + error ---------------------------------------------------------------------->
  ag --smart-case --only-matching --nogroup --nonumbers --all-text --word-regexp -p $AGIGNORE "$RE" $FILES | \
  #                                     <-- generate sed command .... <------------------------------ get correction word ----------------------------->     <------------------------------- match case -------------------------------------------> <-----replace : by / and add word boundary------> ...finalize sed command>
  sed -e 's/\(\S*\):\([[:alnum:]]*\)$/  echo "sed -i \x27s\/"$( echo "\2:$(ag --nonumbers --ignore-case --word-regexp \2 scripts\/spelling.dat | cut -d: -f2)" | sed -r \x27s\/([A-Z]+):(.*)\/\\1:\\U\\2\/; s\/([A-Z][a-z]+):([a-z])\/\\1:\\U\\2\\L\/\x27  | sed -r \x27s\/(\\S\*):\/\\\\b\\1\\\\b\\\/\/\x27)"\/g\x27 \1" /e' | \
  # remove duplicate line
  sort -u
  echo "****"
  echo "Run above commands to fix spelling errors."
  exit 1
else
  exit 0
fi
