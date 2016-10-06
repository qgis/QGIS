#!/bin/bash
###########################################################################
#    sort_include.sh
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


# this sorts and remove duplicates in #include in src and tests folders
# sorts includes in <...> before "..."
# keep #include "ui_..." on top of list
# can skip includes if an order should be kept
# can exclude directories (hard-copies of external libraries)

SORTING=false
FILE1="sort_include_1.tmp"
FILE2="sort_include_2.tmp"
FILE3="sort_include_3.tmp"

# files not to be sorted (leads to compile errors otherwise)
DoNotSort="(sqlite3.h)|(spatialite.h)"

for file in $(find . \
 ! -path "./src/app/gps/qwtpolar-*" \
 ! -path "./src/core/gps/qextserialport/*" \
 ! -path "./src/plugins/grass/qtermwidget/*" \
 ! -path "./src/astyle/*" \
 ! -path "./python/ext-libs/*" \
 ! -path "./src/providers/spatialite/qspatialite/*" \
 ! -path "./src/plugins/dxf2shp_converter/dxflib/src/*" \
 ! -path "./src/plugins/globe/osgEarthQt/*" \
 ! -path "./src/plugins/globe/osgEarthUtil/*" \
 -regex "./src/\(.+/\)*.*\.\(h\|cpp\)" -type f \
 -or -regex "./tests/\(.+/\)*.*\.\(h\|cpp\)" -type f )
do
  echo "$file"
  touch $FILE1
  while IFS= read -r line
  do
    if [[ "$line" =~ ^[[:space:]]*"#"include ]] && [[ ! "$line" =~ $DoNotSort ]]; then
      if ! $SORTING; then
        touch $FILE2
        touch $FILE3
      fi
      SORTING=true
      if [[ "$line" =~ ^"#"include[[:space:]]*\"ui_ ]]; then
        echo "$line" >> $FILE1  # keep ui_ on top of list
      elif [[ "$line" =~ ^"#"include[[:space:]]*\<[^[:space:]]+\> ]]; then
	    echo "$line" >> $FILE2  # include <...>
	  else
	    echo "$line" >> $FILE3  # include "..."
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
  if $SORTING; then
	sort -u $FILE2 >> $FILE1
	sort -u $FILE3 >> $FILE1
	SORTING=false
  fi
  mv $FILE1 $file
  rm -f $FILE1 $FILE2 $FILE3
done

