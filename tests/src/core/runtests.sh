#!/bin/bash
#***************************************************************************
#    runtests.sh
#    --------------------------------------
#   Date                 : Sun Sep 16 12:21:45 AKDT 2007
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

#set -x
LIST=`ls -lah |grep rwxr-xr-x |grep -v ^d |grep -v pl$ |grep -v ~$ |grep -v .sh$ |awk '{print $8}'|awk '$1=$1' RS=`

TOTALEXES=0
TOTALFAILED=0
TOTALPASSED=0
TOTALSKIPPED=0
for FILE in $LIST; 
do 
  RESULT=`./${FILE} | tail -2 |head -1`
  PASSED=`echo ${RESULT} | awk '{print $2}'`
  FAILED=`echo ${RESULT} | awk '{print $4}'`
  SKIPPED=`echo ${RESULT} | awk '{print $6}'`
  TOTALFAILED=`expr $TOTALFAILED + $FAILED`
  TOTALPASSED=`expr $TOTALPASSED + $PASSED`
  TOTALSKIPPED=`expr $TOTALSKIPPED + $SKIPPED`
  TOTALEXES=`expr $TOTALEXES + 1`
done
  echo "-------------------------------"
  echo "TOTAL TESTS  : ${TOTALEXES}"
  echo "-------------------------------"
  echo "TOTAL TEST CASES PASSED  : ${TOTALPASSED}"
  echo "TOTAL TEST CASES FAILED  : ${TOTALFAILED}"
  echo "TOTAL TEST CASES SKIPPED : ${TOTALSKIPPED}"
  echo "-------------------------------"
