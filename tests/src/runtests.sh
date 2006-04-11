#!/bin/bash
#set -x
DIRS=`ls -1F | grep '/$'`
OUTFILE=/tmp/qgistest.`date +%d%h%Y_%H%M`.html
TOTALDIRS=0
TOTALEXES=0
TOTALFAILED=0
TOTALPASSED=0
TOTALSKIPPED=0
echo "<html>" > ${OUTFILE}
echo "<head>" >> ${OUTFILE}
echo "<style type="text/css">" >> ${OUTFILE}
echo "body {background: #5899DB}" >> ${OUTFILE}
echo ".unitSummary {background: #EFEFEF; width: 20em; float:left;}" >> ${OUTFILE}
echo ".moduleSummary {background: #EFEFEF; font-weight: bold; width: 20em; float:left;}" >> ${OUTFILE}
echo ".totalExes {background: #EFEFEF; width: 5em; text-align: center; float:left;}" >> ${OUTFILE}
echo ".totalPasses {background: #EFEFEF; width: 5em; text-align: center; float:left;}" >> ${OUTFILE}
echo ".totalFails {background: #EFEFEF; width: 5em; text-align: center; float:left;}" >> ${OUTFILE}
echo ".totalSkipped {background: #EFEFEF; width: 5em; text-align: center; float:left;}" >> ${OUTFILE}
echo ".{background: #EFEFEF; width: 15em;}" >> ${OUTFILE}
echo "</style>" >> ${OUTFILE}
echo "</head>" >> ${OUTFILE}
echo "<body>" >> ${OUTFILE}
echo "<h1>QGIS Unit Tests : `date +'%d %h %Y : %H%M'`</h1>" >> ${OUTFILE}
for DIR in $DIRS
do
  echo "<div class='module'>"  >> ${OUTFILE}
  echo "<div class='moduleHeader'>Module : ${DIR}</div><br/>" >> ${OUTFILE}
  TOTALDIREXES=0
  TOTALDIRFAILED=0
  TOTALDIRPASSED=0
  TOTALDIRSKIPPED=0
  LIST=`find $DIR -maxdepth 1 -type f -perm +111  | egrep -v '(\.sh$|\.pl$)'`
  for FILE in $LIST
  do 
    RESULT=`${FILE} | grep '^Totals:'`
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
    echo "<div class="unitSummary">$FILE</div>" >> ${OUTFILE}
    echo "<div class="totalExes">&nbsp;</div>" >> ${OUTFILE}
    echo "<div class="totalPasses">$PASSED</div>" >> ${OUTFILE}
    echo "<div class="totalFails">$FAILED</div>" >> ${OUTFILE}
    echo "<div class="totalSkipped">$SKIPPED</div>" >> ${OUTFILE}
    echo "<br/>" >> ${OUTFILE}
  done
  TOTALDIRS=`expr $TOTALDIRS + 1`
  echo "-------------------------------"
  echo "MODULE : $DIR"
  echo "-------------------------------"
  echo "MODULE TESTS  : ${TOTALDIREXES}"
  echo "MODULE TEST CASES PASSED  : ${TOTALDIRPASSED}"
  echo "MODULE TEST CASES FAILED  : ${TOTALDIRFAILED}"
  echo "MODULE TEST CASES SKIPPED : ${TOTALDIRSKIPPED}"
  echo "<div class="moduleSummary">Totals:</div>" >> ${OUTFILE}
  echo "<div class="totalExes">$TOTALDIREXES</div>" >> ${OUTFILE}
  echo "<div class="totalPasses">$TOTALDIRPASSED</div>" >> ${OUTFILE}
  echo "<div class="totalFails">$TOTALDIRFAILED</div>" >> ${OUTFILE}
  echo "<div class="totalSkipped">$TOTALDIRSKIPPED</div>" >> ${OUTFILE}
  echo "<br/>" >> ${OUTFILE}
  echo "</div><!--end of module -->" >> ${OUTFILE}
done
echo "-------------------------------"
echo "       TOTALS :"
echo "-------------------------------"
echo "TOTAL TESTS  : ${TOTALEXES}"
echo "TOTAL TEST CASES PASSED  : ${TOTALPASSED}"
echo "TOTAL TEST CASES FAILED  : ${TOTALFAILED}"
echo "TOTAL TEST CASES SKIPPED : ${TOTALSKIPPED}"
echo "<div class="suiteSummary">Totals:</div>" >> ${OUTFILE}
echo "<div class="totalExes">$TOTALEXES</div>" >> ${OUTFILE}
echo "<div class="totalPasses">$TOTALPASSED</div>" >> ${OUTFILE}
echo "<div class="totalFails">$TOTALFAILED</div>" >> ${OUTFILE}
echo "<div class="totalSkipped">$TOTALSKIPPED</div>" >> ${OUTFILE}
echo "<br/>" >> ${OUTFILE}
echo "</body>" >> ${OUTFILE}
echo "</html>" >> ${OUTFILE}
firefox ${OUTFILE}
