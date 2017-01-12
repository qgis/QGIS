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

# -i: enter interactive mode to fix errors
# optional argument: list of files to be checked


DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

AGIGNORE=${DIR}/.agignore

# ARGUMENTS
INTERACTIVE=YES
while getopts ":r" opt; do
  case $opt in
    r)
      echo "interactive mode turned off" >&2
      INTERACTIVE=NO
      ;;
    \?)
      echo "Invalid option: -$OPTARG" >&2
      exit 1
      ;;
  esac
done
shift $(expr $OPTIND - 1)

if [ ! $# -eq 0 ]; then
  EXCLUDE=$(cat $AGIGNORE | sed -e 's/\s*#.*$//' -e '/^\s*$/d' | tr '\n' '|' | sed -e 's/|$//')
  INPUTFILES=$(echo $@ | tr -s '[[:blank:]]' '\n' | egrep -iv "$EXCLUDE" | tr '\n' ' ' )
  echo "Running spell check on files: $INPUTFILES"
else
  INPUTFILES="."
fi

SPELLOK='(#\s*spellok|<!--#\s*spellok-->)$'

# split into several files to avoid too long regexes
SPLIT=4
GNUPREFIX=
if [[ "$OSTYPE" =~ darwin* ]]; then
  GNUPREFIX=g
fi

${GNUPREFIX}split --number=l/$SPLIT --numeric-suffixes=1 --suffix-length=1 --additional-suffix=~ ${DIR}/spelling.dat spelling


for ((I=1;I<=$SPLIT;I++)) ; do
  SPELLFILE=spelling$I~;

  # This will try to look for mispelling within larger words.
  # Condition is hard to explain in words.
  # You can test it here: https://regex101.com/r/7kznVA/9
  # extra words that should not be checked in longer words
  WHOLEWORDS=$(echo "("; perl -ne 'print if not /^(\w)(\w)\w{2,}(\w)(\w):(\2\2|\1(?:(?!\1)\w)|(?:(?!\1)\w)\2|(?:(?!\1)\w)(?:(?!\1)\w)|\2\1)\w*(\3\3|(?:(?!\4)\w)(?:(?!\3)\4)|\3(?:(?!\4).)|(?:(?!\4)\w)(?:(?!\4)\w)|\4\3)(?!:\*)$/' $SPELLFILE | cut -d: -f1 |  tr '\n' '\|' | sed -e 's/|$//'; echo ")")
  INWORDS=$(   echo "("; perl -ne 'print if     /^(\w)(\w)\w{2,}(\w)(\w):(\2\2|\1(?:(?!\1)\w)|(?:(?!\1)\w)\2|(?:(?!\1)\w)(?:(?!\1)\w)|\2\1)\w*(\3\3|(?:(?!\4)\w)(?:(?!\3)\4)|\3(?:(?!\4).)|(?:(?!\4)\w)(?:(?!\4)\w)|\4\3)(?!:\*)$/' $SPELLFILE | cut -d: -f1 |  tr '\n' '\|' | sed -e 's/|$//'; echo ")")

  FILE=$INPUTFILES  # init with input files (if ag is run with single file, file path is now in output)
  COMMANDS=""
  ERRORFOUND=NO
  while read -u 3 -r LINE; do
	echo "$LINE"
	ERRORFOUND=YES
	if [[ "$INTERACTIVE" =~ YES ]]; then
      NOCOLOR=$(echo "$LINE" | ${GNUPREFIX}sed -r 's/\x1B\[([0-9]{1,2}(;[0-9]{1,2})?)?[m|K]//g')
      if [[ "$NOCOLOR" =~ ^[[:alnum:]][[:alnum:]\/\._-]+$ ]]; then
        FILE=$NOCOLOR
      fi
      if [[ "$NOCOLOR" =~ ^[0-9]+: ]]; then
        if [[ -z $FILE ]]; then
          echo "Error: no file"
          exit 1
        fi
        NUMBER=$(echo "$NOCOLOR" | cut -d: -f1)
        ERROR=$(echo "$LINE" | ${GNUPREFIX}sed -r 's/^.*\x1B\[30;43m(.*?)\x1B\[0m.*$/\1/')
        CORRECTION=$(ag --nonumbers --ignore-case --word-regexp "$ERROR" ${DIR}/spelling.dat | cut -d: -f2)

        SPELLOKSTR='//#spellok'
        if [[ "$FILE" =~ \.(txt|html|htm)$ ]]; then
          SPELLOKSTR='<!--#spellok-->'
        fi
        if [[ "$FILE" =~ \.(h|cpp|sip)$ ]]; then
          if [[ "$NOCOLOR" =~ ^\s*(\/*)|(\/\/) ]]; then
            SPELLOKSTR='#spellok'
          fi
        fi
        if [[ "$FILE" =~ \.(py)$ ]]; then
	      SPELLOKSTR='#spellok'
	    fi

        echo ""
        echo -e "  \x1B[4mr\x1B[0meplace by \x1B[33m$CORRECTION\x1B[0m"
        echo -e "  \x1B[4ma\x1B[0mppend \x1B[33m$SPELLOKSTR\x1B[0m at the end of the line to avoid spell check on this line"
        echo -e "  en\x1B[4mt\x1B[0mer your own correction"
        echo -e "  ignore and \x1B[4mc\x1B[0montinue"
        echo -e "  ignore and \x1B[4me\x1B[0mxit"

        while read -n 1 n; do
          echo ""
          case $n in
              r)
                MATCHCASE="$ERROR:$CORRECTION"
                CORRECTIONCASE=$(echo "$MATCHCASE" | ${GNUPREFIX}sed -r 's/([A-Z]+):(.*)/\U\2/;s/([A-Z][a-z]+):([a-z])/\U\2\L/')
                echo -e "replacing \x1B[33m$ERROR\x1B[0m by \x1B[33m$CORRECTION\x1B[0m in \x1B[33m$FILE\x1B[0m at line \x1B[33m$NUMBER\x1B[0m"
                ${GNUPREFIX}sed -i "${NUMBER}s/$ERROR/$CORRECTION/g" $FILE
                break
                ;;
              a)
                echo -e "appending \x1B[33m$SPELLOKSTR\x1B[0m to \x1B[33m$FILE\x1B[0m at line \x1B[33m$NUMBER\x1B[0m"
                SPELLOKSTR=$(echo "$SPELLOKSTR" | ${GNUPREFIX}sed -r 's/\//\\\//g')
                ${GNUPREFIX}sed -i "${NUMBER}s/\$/  $SPELLOKSTR/" $FILE
                break
                ;;
              t)
                echo "Enter the correction: "
                read CORRECTION
                MATCHCASE="$ERROR:$CORRECTION"
                CORRECTIONCASE=$(echo "$MATCHCASE" | ${GNUPREFIX}sed -r 's/([A-Z]+):(.*)/\U\2/;s/([A-Z][a-z]+):([a-z])/\U\2\L/')
                echo -e "replacing \x1B[33m$ERROR\x1B[0m by \x1B[33m$CORRECTION\x1B[0m in \x1B[33m$FILE\x1B[0m at line \x1B[33m$NUMBER\x1B[0m"                sed -i "${NUMBER}s/$ERROR/$CORRECTION/g" $FILE
                break
                ;;
              c)
                break
                ;;
              e)
                exit 1
                ;;
              *) invalid option;;
          esac
        done

      fi
      if [[ "$NOCOLOR" =~ ^\s*$ ]]; then
        FILE=""
      fi
    fi
  done 3< <(unbuffer ag --smart-case --all-text --nopager --color-match "30;43" --numbers --nomultiline --word-regexp -p $AGIGNORE "${WHOLEWORDS}"'(?!.*'"${SPELLOK}"')' $INPUTFILES ; \
           unbuffer ag --smart-case --all-text --nopager --color-match "30;43" --numbers --nomultiline               -p $AGIGNORE "${INWORDS}"'(?!.*'"${SPELLOK}"')'    $INPUTFILES )

  rm $SPELLFILE

  if [[ "$ERRORFOUND" =~ YES ]]; then
    echo -e "\x1B[1msome errors have been found.\x1B[0m"
    exit 1
  else
    exit 0
  fi
done
exit
