#!/bin/bash
#set -x
DIRS=`ls -1F |grep / |sed 's/\///g'`

TOTALDIRS=0
TOTALEXES=0
TOTALFAILED=0
TOTALPASSED=0
TOTALSKIPPED=0
for DIR in $DIRS
do
  TOTALDIREXES=0
  TOTALDIRFAILED=0
  TOTALDIRPASSED=0
  TOTALDIRSKIPPED=0
  LIST=`ls -lah $DIR |grep rwxr-xr-x |grep -v ^d |grep -v pl$ |grep -v ~$ |grep -v .sh$ |awk '{print $8}'|awk '$1=$1' RS=`
  for FILE in $LIST
  do 
    RESULT=`$DIR/${FILE} | tail -2 |head -1` #TODO maybe just grep for 'Totals'
    PASSED=`echo ${RESULT} | awk '{print $2}'`
    FAILED=`echo ${RESULT} | awk '{print $4}'`
    SKIPPED=`echo ${RESULT} | awk '{print $6}'`
    TOTALDIRFAILED=`expr $TOTALDIRFAILED + $FAILED`
    TOTALDIRPASSED=`expr $TOTALDIRPASSED + $PASSED`
    TOTALDIRSKIPPED=`expr $TOTALDIRSKIPPED + $SKIPPED`
    TOTALDIREXES=`expr $TOTALDIREXES + 1`
    TOTALFAILED=`expr $TOTALFAILED + $FAILED`
    TOTALPASSED=`expr $TOTALPASSED + $PASSED`
    TOTALSKIPPED=`expr $TOTALSKIPPED + $SKIPPED`
    TOTALEXES=`expr $TOTALEXES + 1`
  done
  TOTALDIRS=`expr $TOTALDIRS + 1`
  echo "-------------------------------"
  echo "MODULE : $DIR"
  echo "-------------------------------"
  echo "MODULE TESTS  : ${TOTALDIREXES}"
  echo "MODULE TEST CASES PASSED  : ${TOTALDIRPASSED}"
  echo "MODULE TEST CASES FAILED  : ${TOTALDIRFAILED}"
  echo "MODULE TEST CASES SKIPPED : ${TOTALDIRSKIPPED}"
done
echo "-------------------------------"
echo "       TOTALS :"
echo "-------------------------------"
echo "TOTAL TESTS  : ${TOTALEXES}"
echo "TOTAL TEST CASES PASSED  : ${TOTALPASSED}"
echo "TOTAL TEST CASES FAILED  : ${TOTALFAILED}"
echo "TOTAL TEST CASES SKIPPED : ${TOTALSKIPPED}"
