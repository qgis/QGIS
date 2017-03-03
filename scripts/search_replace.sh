#!/usr/bin/env bash
###########################################################################
#    search_replace.sh
#    ---------------------
#    Date                 : February 2017
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

# usage: search_replace [options] 'search' ['replace']

DIR=$(git rev-parse --show-toplevel)/scripts/spell_check

AGIGNORE=${DIR}/.agignore
PATH=${DIR}/src

# GNU prefix command for mac os support (gsed, gsplit)
GP=
if [[ "$OSTYPE" =~ darwin* ]]; then
  GP=g
fi

# ARGUMENTS
INTERACTIVE=$(tty -s && echo YES || echo NO)
DEBUG=NO
OUTPUTLOG=""
while getopts ":p:" opt; do
  case $opt in
    p)
      PATH=$OPTARG
      ;;
    \?)
      echo "Invalid option: -$OPTARG" >&2
      exit 1
      ;;
  esac
done
shift $(expr $OPTIND - 1)

if [ ! $# -eq 0 ]; then
  echo 'no text to search'
  exit 1
fi

SEARCH_PATTERN="$1"
MAIN_REPLACE="$2"


# global replace variables (dictionary)
declare -A GLOBREP_ALLFILES=()
declare -A GLOBREP_CURRENTFILE=()
declare -A GLOBREP_IGNORE=()
declare -A GLOBREP_HISTORY=()


while read -u 3 -r LINE; do
  echo "$LINE"
  SEARCHFOUND=YES
  NOCOLOR=$(echo "$LINE" | ${GP}sed -r 's/\x1B\[([0-9]{1,2}(;[0-9]{1,2})?)?[m|K]//g')
  if [[ "$NOCOLOR" =~ ^[[:alnum:]][[:alnum:]\/\._-]+$ ]]; then
    FILE=$NOCOLOR
    GLOBREP_CURRENTFILE=()
  fi
  if [[ "$NOCOLOR" =~ ^[0-9]+: ]]; then
    if [[ -z $FILE ]]; then
      echo "*** ERROR: no file"
      exit 1
    fi
    NUMBER=$(echo "$NOCOLOR" | cut -d: -f1)
    SEARCHLINE=$(echo "$NOCOLOR" | cut -d: -f2)
    SEARCH=$(echo "$LINE" | ${GP}sed -r 's/^.*?\x1B\[30;43m(.*?)\x1B\[0m.*$/\1/')
    SEARCHNOCOLOR=$(echo "$SEARCH" | ${GP}sed -r 's/\x1B\[([0-9]{1,2}(;[0-9]{1,2})?)?[m|K]//g')

    if [[ "$SEARCHNOCOLOR" =~ ^[[:digit:]]+: ]]; then
      echo "*** ERROR: could not find search in $LINE" >&2
    else

      # Skip global replace
      if [[ !  -z  ${GLOBREP_ALLFILES["$SEARCH"]} ]]; then
        echo -e "replace \x1B[33m$SEARCH\x1B[0m by \x1B[33m$REPLACE\x1B[0m in \x1B[33m$FILE\x1B[0m"
        ${GP}sed -i -r "s/$SEARCH/$REPLACE/g" $FILE
        continue
      elif [[ ( ! -z  ${GLOBREP_CURRENTFILE["$SEARCH"]} ) || ( ! -z  ${GLOBREP_IGNORE["$SEARCH"]} ) ]]; then
        echo "skipping occurrence"
        continue
      else
        # Display menu
        echo "***"
        echo -e "SEARCH found: \x1B[31m$SEARCH\x1B[0m"
        if [[ ! -z $MAIN_REPLACE ]]
          echo -e "  r) \x1B[4mr\x1B[0meplace by \x1B[33m$MAIN_REPLACE\x1B[0m at line $NUMBER"
          echo -e "  f) replace all occurrences by \x1B[33m$MAIN_REPLACE\x1B[0m in current \x1B[4mf\x1B[0mile"
          echo -e "  a) replace all occurrences by \x1B[33m$MAIN_REPLACE\x1B[0m in \x1B[4ma\x1B[0mll files"
        fi
        echo -e "  t) \x1B[4mt\x1B[0mype your own correction"
        echo -e "  c) skip and \x1B[4mc\x1B[0montinue"
        echo -e "  e) \x1B[4me\x1B[0mxit"

        while read -n 1 n; do
          echo ""
          case $n in
              r)
                echo -e "replacing \x1B[33m$SEARCH\x1B[0m by \x1B[33m$CORRECTIONCASE\x1B[0m in \x1B[33m$FILE\x1B[0m at line \x1B[33m$NUMBER\x1B[0m"
                ${GP}sed -i "${NUMBER}s/$SEARCH/$CORRECTIONCASE/g" $FILE
                break
                ;;
              f)
                GLOBREP_CURRENTFILE+=(["$SEARCH"]=1)
                echo -e "replacing \x1B[33m$SEARCH\x1B[0m by \x1B[33m$CORRECTIONCASE\x1B[0m in \x1B[33m$FILE\x1B[0m"
                ${GP}sed -i -r "/${SPELLOKRX}/! s/$SEARCH/$CORRECTIONCASE/g" $FILE
                break
                ;;
              a)
                GLOBREP_CURRENTFILE+=(["$SEARCH"]=1)
                GLOBREP_ALLFILES+=(["$SEARCH"]=1)
                echo -e "replace \x1B[33m$SEARCH\x1B[0m by \x1B[33m$CORRECTIONCASE\x1B[0m in \x1B[33m$FILE\x1B[0m"
                ${GP}sed -i -r "/${SPELLOKRX}/! s/$SEARCH/$CORRECTIONCASE/g" $FILE
                break
                ;;
              p)
                echo -e "appending \x1B[33m$SPELLOKSTR\x1B[0m to \x1B[33m$FILE\x1B[0m at line \x1B[33m$NUMBER\x1B[0m"
                ${GP}sed -i "${NUMBER}s/\$/  $SPELLOKSTR_ESC/" $FILE
                break
                ;;
              t)
                echo "Enter the correction: "
                read CORRECTION
                MATCHCASE="$SEARCH:$CORRECTION"
                CORRECTIONCASE=$(echo "$MATCHCASE" | ${GP}sed -r 's/([A-Z]+):(.*)/\1:\U\2/; s/([A-Z][a-z]+):([a-z])/\1:\U\2\L/' | cut -d: -f2)
                echo -e "replacing \x1B[33m$SEARCH\x1B[0m by \x1B[33m$CORRECTIONCASE\x1B[0m in \x1B[33m$FILE\x1B[0m at line \x1B[33m$NUMBER\x1B[0m"
                ${GP}sed -i "${NUMBER}s/$SEARCH/$CORRECTIONCASE/g" $FILE
                break
                ;;
              c)
                break
                ;;
              o)
                GLOBREP_IGNORE+=(["$SEARCH"]=1)
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
done 3< <(
  unbuffer ag --all-text --nopager --color-match "30;43" --numbers --nomultiline --case-sensitive -p $AGIGNORE "$SEARCH_PATTERN" $PATH
)


exit 0
