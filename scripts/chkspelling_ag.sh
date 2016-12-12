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


RE=$(cut -d: -f1 scripts/spelling.dat |  tr '\n' '\|' | sed -e 's/|$//')
if [ ! $# -eq 0 ]; then
  EXCLUDE=$(cat ci/travis/.agignore | sed -e 's/\s*#.*$//' -e '/^\s*$/d' | tr '\n' '|' | sed -e 's/|$//')
  FILES=$(echo $@ | tr -s '[[:blank:]]' '\n' | egrep -iv "$EXCLUDE" | tr '\n' ' ' )
  echo "Running spell check on files: $FILES"
else
  FILES="."
fi


exec 5>&1
OUTPUT=$(ag --smart-case --all-text --nopager --numbers --word-regexp --path-to-ignore scripts/.agignore "$RE" $FILES |tee /dev/fd/5)


if [[ !  -z  $OUTPUT  ]]; then
  echo "Spelling errors have been found"
  exit 1
else
  exit 0
fi
