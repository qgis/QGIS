#!/bin/bash
#***************************************************************************
#    test_suite_builder.sh
#    --------------------------------------
#   Date                 : Sun Sep 16 12:22:12 AKDT 2007
#   Copyright            : (C) 2007 by Gary E. Sherman
#   Email                : sherman at mrcc dot com
#***************************************************************************
#*                                                                         *
#*   This program is free software; you can redistribute it and/or modify  *
#*   it under the terms of the GNU General Public License as published by  *
#*   the Free Software Foundation; either version 2 of the License, or     *
#*   (at your option) any later version.                                   *
#*                                                                         *
#***************************************************************************/

LIST=`ls ../../../src/core/ |grep .cpp |grep ^qgs |grep -v ~$ |grep -v moc.cpp$ | sed 's/.cpp//g' |awk '$1=$1' RS= |sort`
for FILE in $LIST
do 
  CLASSNAME=`grep -o "::Qgs[A-Za-z0-9]*(" ../../../src/core/${FILE}.cpp |head -1 | sed 's/:://g'| sed 's/(//g'`
  ./test_builder.pl $CLASSNAME
  #svn add test${FILE}.cpp
done
make install
