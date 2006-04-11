#!/bin/bash
#set -x
DIRS=`ls -1F | grep '/$'`
OUTFILE=/tmp/qgistest.`date +%d%h%Y_%Hh%M`.html
TOTALDIRS=0
TOTALEXES=0
TOTALFAILED=0
TOTALPASSED=0
TOTALSKIPPED=0
echo "<html>" > ${OUTFILE}
echo "<head>" >> ${OUTFILE}
echo "<style type="text/css">" >> ${OUTFILE}
echo "body {background: white}" >> ${OUTFILE}
echo "h1 {text-align: center;}" >> ${OUTFILE}
echo "h2 {text-align: center;}" >> ${OUTFILE}
echo ".module {background: #FF9D4D; width: 32em;}" >> ${OUTFILE}
echo ".suiteSummary {background: #F9E5D5; font-weight: bold; width: 20em; float:left;}" >> ${OUTFILE}
echo ".moduleHeader {background: #EB6E08; font-size: bigger; font-weight: bold; text-align: center; width: 32em; margin-top: 1em; margin-bottom: 0em; }" >> ${OUTFILE}
echo ".moduleSummary {background: #FCBA82; font-weight: bold; width: 20em; float:left;}" >> ${OUTFILE}
echo ".unitSummary {background: #EFEFEF; width: 20em; float:left;}" >> ${OUTFILE}
echo ".unitSummaryFailed {background: #EFEFEF; color: crimson; width: 20em; float:left;}" >> ${OUTFILE}
echo ".unitTotalExes {background: #EFEFEF; width: 3em; text-align: center; float:left;}" >> ${OUTFILE}
echo ".unitTotalPasses {background: #EFEFEF; width: 3em; text-align: center; float:left;}" >> ${OUTFILE}
echo ".unitTotalFails {background: #EFEFEF; width: 3em; text-align: center; float:left;}" >> ${OUTFILE}
echo ".unitTotalSkipped {background: #EFEFEF; width: 3em; text-align: center; float:left;}" >> ${OUTFILE}
echo ".moduleTotalExes {background: #FCBA82; width: 3em; text-align: center; float:left;}" >> ${OUTFILE}
echo ".moduleTotalPasses {background: #FCBA82; width: 3em; text-align: center; float:left;}" >> ${OUTFILE}
echo ".moduleTotalFails {background: #FCBA82; width: 3em; text-align: center; float:left;}" >> ${OUTFILE}
echo ".moduleTotalSkipped {background: #FCBA82; width: 3em; text-align: center; float:left;}" >> ${OUTFILE}
echo ".suiteTotalExes {background: #FF9946; width: 3em; text-align: center; float:left;}" >> ${OUTFILE}
echo ".suiteTotalPasses {background: #FF9946; width: 3em; text-align: center; float:left;}" >> ${OUTFILE}
echo ".suiteTotalFails {background: #FF9946; width: 3em; text-align: center; float:left;}" >> ${OUTFILE}
echo ".suiteTotalSkipped {background: #FF9946; width: 3em; text-align: center; float:left;}" >> ${OUTFILE}
echo "</style>" >> ${OUTFILE}
echo "</head>" >> ${OUTFILE}
echo "<body>" >> ${OUTFILE}
echo "<h1>QGIS Unit Tests</h1>" >> ${OUTFILE}
echo "<h2>`date +'%d %h %Y : %H%M'`</h2>" >> ${OUTFILE}
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
    echo "<div class="unitTotalExes">&nbsp;</div>" >> ${OUTFILE}
    echo "<div class="unitTotalPasses">$PASSED</div>" >> ${OUTFILE}
    echo "<div class="unitTotalFails">$FAILED</div>" >> ${OUTFILE}
    echo "<div class="unitTotalSkipped">$SKIPPED</div>" >> ${OUTFILE}
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
  echo "<div class="moduleTotalExes">$TOTALDIREXES</div>" >> ${OUTFILE}
  echo "<div class="moduleTotalPasses">$TOTALDIRPASSED</div>" >> ${OUTFILE}
  echo "<div class="moduleTotalFails">$TOTALDIRFAILED</div>" >> ${OUTFILE}
  echo "<div class="moduleTotalSkipped">$TOTALDIRSKIPPED</div>" >> ${OUTFILE}
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
echo "<div class='moduleHeader'>Global Summary:</div><br/>" >> ${OUTFILE}
echo "<div class="suiteSummary">Totals:</div>" >> ${OUTFILE}
echo "<div class="suiteTotalExes">$TOTALEXES</div>" >> ${OUTFILE}
echo "<div class="suiteTotalPasses">$TOTALPASSED</div>" >> ${OUTFILE}
echo "<div class="suiteTotalFails">$TOTALFAILED</div>" >> ${OUTFILE}
echo "<div class="suiteTotalSkipped">$TOTALSKIPPED</div>" >> ${OUTFILE}
echo "<br/>" >> ${OUTFILE}
echo "</body>" >> ${OUTFILE}
echo "</html>" >> ${OUTFILE}
firefox ${OUTFILE}
