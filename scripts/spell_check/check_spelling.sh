#!/bin/bash
###########################################################################
#    checkk_spelling.sh
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

# -r: deactivate interactive mode to fix errors
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

# prefix command for mac os support (gsed, gsplit)
GNUPREFIX=
if [[ "$OSTYPE" =~ darwin* ]]; then
  GNUPREFIX=g
fi

# regex to find escape string
SPELLOKRX='(#\s*spellok|<!--\s*#\s*spellok\s*-->)$'

# split into several files to avoid too long regexes
SPLIT=4

${GNUPREFIX}split --number=l/$SPLIT --numeric-suffixes --suffix-length=1 --additional-suffix=~ ${DIR}/spelling.dat spelling

# global replace variables (dictionary)
declare -A GLOBREP_ALLFILES=()
declare -A GLOBREP_CURRENTFILE=()
declare -A GLOBREP_IGNORE=()

for ((I=0;I<$SPLIT;I++)) ; do
  SPELLFILE=spelling$I~;

  # This will try to look for misspelling within larger words.
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
        GLOBREP_CURRENTFILE=()
      fi
      if [[ "$NOCOLOR" =~ ^[0-9]+: ]]; then
        if [[ -z $FILE ]]; then
          echo "Error: no file"
          exit 1
        fi
        NUMBER=$(echo "$NOCOLOR" | cut -d: -f1)
        ERROR=$(echo "$LINE" | ${GNUPREFIX}sed -r 's/^.*?\x1B\[30;43m(.*?)\x1B\[0m.*$/\1/')
        CORRECTION=$(ag --nonumbers --ignore-case "^$ERROR:" ${DIR}/spelling.dat | cut -d: -f2)
        # Match case
        MATCHCASE="$ERROR:$CORRECTION"
        CORRECTIONCASE=$(echo "$MATCHCASE" | ${GNUPREFIX}sed -r 's/([A-Z]+):(.*)/\1:\U\2/; s/([A-Z][a-z]+):([a-z])/\1:\U\2\L/' | cut -d: -f2)

        # Skip global replace
        if [[ !  -z  ${GLOBREP_ALLFILES["$ERROR"]} ]]; then
          echo -e "replace \x1B[33m$ERROR\x1B[0m by \x1B[33m$CORRECTIONCASE\x1B[0m in \x1B[33m$FILE\x1B[0m"
          ${GNUPREFIX}sed -i -r "/${SPELLOKRX}/! s/$ERROR/$CORRECTIONCASE/g" $FILE
          continue

        elif [[ -z  ${GLOBREP_CURRENTFILE["$ERROR"]} ]] && [[ -z  ${GLOBREP_IGNORE["$ERROR"]} ]]; then
          # escape string
          SPELLOKSTR='//#spellok'
          if [[ "$FILE" =~ \.(txt|html|htm|dox)$ ]]; then
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
          SPELLOKSTR_ESC=$(echo "$SPELLOKSTR" | ${GNUPREFIX}sed -r 's/\//\\\//g')

          # Display menu
          echo "***"
          echo -e "Error found: \x1B[31m$ERROR\x1B[0m"
          echo -e "  \x1B[4mr\x1B[0meplace by \x1B[33m$CORRECTIONCASE\x1B[0m at line $NUMBER"
          echo -e "  replace all occurrences by \x1B[33m$CORRECTIONCASE\x1B[0m in current \x1B[4mf\x1B[0mile"
          echo -e "  replace all occurrences by \x1B[33m$CORRECTIONCASE\x1B[0m in \x1B[4ma\x1B[0mll files"
          echo -e "  a\x1B[4mp\x1B[0mpend \x1B[33m$SPELLOKSTR\x1B[0m at the end of the line $NUMBER to avoid spell check on this line"
          echo -e "  en\x1B[4mt\x1B[0mer your own correction"
          echo -e "  skip and \x1B[4mc\x1B[0montinue"
          echo -e "  skip all \x1B[4mo\x1B[0mccurences and continue"
          echo -e "  ignore and \x1B[4me\x1B[0mxit"

          while read -n 1 n; do
            echo ""
            case $n in
                r)
                  echo -e "replacing \x1B[33m$ERROR\x1B[0m by \x1B[33m$CORRECTIONCASE\x1B[0m in \x1B[33m$FILE\x1B[0m at line \x1B[33m$NUMBER\x1B[0m"
                  ${GNUPREFIX}sed -i "${NUMBER}s/$ERROR/$CORRECTIONCASE/g" $FILE
                  break
                  ;;
                f)
                  GLOBREP_CURRENTFILE+=(["$ERROR"]=1)
                  echo -e "replacing \x1B[33m$ERROR\x1B[0m by \x1B[33m$CORRECTIONCASE\x1B[0m in \x1B[33m$FILE\x1B[0m"
                  ${GNUPREFIX}sed -i -r "/${SPELLOKRX}/! s/$ERROR/$CORRECTIONCASE/g" $FILE
                  break
                  ;;
                a)
                  GLOBREP_CURRENTFILE+=(["$ERROR"]=1)
                  GLOBREP_ALLFILES+=(["$ERROR"]=1)
                  echo -e "replace \x1B[33m$ERROR\x1B[0m by \x1B[33m$CORRECTIONCASE\x1B[0m in \x1B[33m$FILE\x1B[0m"
                  ${GNUPREFIX}sed -i -r "/${SPELLOKRX}/! s/$ERROR/$CORRECTIONCASE/g" $FILE
                  break
                  ;;
                p)
                  echo -e "appending \x1B[33m$SPELLOKSTR\x1B[0m to \x1B[33m$FILE\x1B[0m at line \x1B[33m$NUMBER\x1B[0m"
                  ${GNUPREFIX}sed -i "${NUMBER}s/\$/  $SPELLOKSTR_ESC/" $FILE
                  break
                  ;;
                t)
                  echo "Enter the correction: "
                  read CORRECTION
                  MATCHCASE="$ERROR:$CORRECTION"
                  CORRECTIONCASE=$(echo "$MATCHCASE" | ${GNUPREFIX}sed -r 's/([A-Z]+):(.*)/\1:\U\2/; s/([A-Z][a-z]+):([a-z])/\1:\U\2\L/' | cut -d: -f2)
                  echo -e "replacing \x1B[33m$ERROR\x1B[0m by \x1B[33m$CORRECTIONCASE\x1B[0m in \x1B[33m$FILE\x1B[0m at line \x1B[33m$NUMBER\x1B[0m"
                  sed -i "${NUMBER}s/$ERROR/$CORRECTIONCASE/g" $FILE
                  break
                  ;;
                c)
                  break
                  ;;
                o)
                  GLOBREP_IGNORE+=(["$ERROR"]="$CORRECTION")
                  break
                  ;;
                e)
                  exit 1
                  ;;
                *) invalid option;;
            esac
          done
        fi
      fi
      if [[ "$NOCOLOR" =~ ^\s*$ ]]; then
        FILE=""
      fi
    fi
  done 3< <(unbuffer ag --smart-case --all-text --nopager --color-match "30;43" --numbers --nomultiline --word-regexp -p $AGIGNORE "${WHOLEWORDS}"'(?!.*'"${SPELLOKRX}"')' $INPUTFILES ; \
           unbuffer ag --smart-case --all-text --nopager --color-match "30;43" --numbers --nomultiline               -p $AGIGNORE "${INWORDS}"'(?!.*'"${SPELLOKRX}"')'    $INPUTFILES )

  rm $SPELLFILE

done

if [[ "$ERRORFOUND" =~ YES ]]; then
  echo -e "\x1B[1msome errors have been found.\x1B[0m"
  exit 1
else
  exit 0
fi

exit
