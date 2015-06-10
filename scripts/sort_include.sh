#!/bin/bash
###########################################################################
#    chkspelling.sh
#    ---------------------
#    Date                 : June 2015
#    Copyright            : (C) 2015 by Denis Rouzaud
#    Email                : denis.rouzaud@gmail.com
###########################################################################
#                                                                         #
#   This program is free software; you can redistribute it and/or modify  #
#   it under the terms of the GNU General Public License as published by  #
#   the Free Software Foundation; either version 2 of the License, or     #
#   (at your option) any later version.                                   #
#                                                                         #
###########################################################################


# this sorts and remove duplicates in #include
# sorts includes in <...> before "..."
# keep #include "ui_..." on top of list

SORTING=false
FILE1=sort1.tmp
FILE2=sort2.tmp
FILE3=sort3.tmp

for file in $(find ./src -type f -regex ".*/.*\.\(h\|cpp\)"); do
  echo "$file"
  touch $FILE1
  while IFS= read -r line
  do
    if [[ "$line" =~ ^[[:space:]]*"#"include ]]; then
      if ! $SORTING; then
        touch $FILE2
        touch $FILE3
      fi
      SORTING=true
      if [[ "$line" =~ ^"#"include[[:space:]]*\"ui_ ]]; then
        echo "$line" >> $FILE1  # keep ui_ on top of list
      elif [[ "$line" =~ ^"#"include[[:space:]]*\<[^[:space:]]+\> ]]; then
	    echo "$line" >> $FILE2
	  else
	    echo "$line" >> $FILE3
	  fi
    else
      if $SORTING; then
        sort -u $FILE2 >> $FILE1
        sort -u $FILE3 >> $FILE1
        rm -f $FILE2 $FILE3
        SORTING=false
      fi
      echo "$line" >> $FILE1
    fi
  done < "$file"
  rm -f $FILE2 $FILE3
  mv $FILE1 $file
  rm -f $FILE1
done

