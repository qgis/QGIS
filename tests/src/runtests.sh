#***************************************************************************
#    runtests.sh
#    --------------------------------------
#   Date                 : Sun Sep 16 12:21:00 AKDT 2007
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
echo "body {background: white;  text-align: center;    min-width: 33em;  }" >> ${OUTFILE}
echo "#wrapper {  margin:0 auto;  width:33em;    text-align: left;  }" >> ${OUTFILE}

echo "h1 {text-align: center;}" >> ${OUTFILE}
echo "h2 {text-align: center;}" >> ${OUTFILE}

echo ".statusPass {background: green; width: 1em; float:left;}" >> ${OUTFILE}
echo ".statusFail {background: crimson; width: 1em; float:left;}" >> ${OUTFILE}

echo ".unitSummary {background: #EFEFEF; width: 20em; float:left;}" >> ${OUTFILE}
echo ".unitTotalExes {background: #EFEFEF; width: 3em; text-align: center; float:left;}" >> ${OUTFILE}
echo ".unitTotalPasses {background: #EFEFEF; width: 3em; text-align: center; float:left;}" >> ${OUTFILE}
echo ".unitTotalFails {background: #EFEFEF; width: 3em; text-align: center; float:left;}" >> ${OUTFILE}
echo ".unitTotalSkipped {background: #EFEFEF; width: 3em; text-align: center; float:left;}" >> ${OUTFILE}

echo ".colSummary {background: #FEB87F; font-weight: bold; width: 21em; float:left;}" >> ${OUTFILE}
echo ".colTotalExes {background: #FEB87F;font-weight: bold; width: 3em; text-align: center; float:left;}" >> ${OUTFILE}
echo ".colTotalPasses {background: #FEB87F;font-weight: bold; width: 3em; text-align: center; float:left;}" >> ${OUTFILE}
echo ".colTotalFails {background: #FEB87F;font-weight: bold; width: 3em; text-align: center; float:left;}" >> ${OUTFILE}
echo ".colTotalSkipped {background: #FEB87F;font-weight: bold; width: 3em; text-align: center; float:left;}" >> ${OUTFILE}

echo ".module {background: #FF9D4D; width: 33em;}" >> ${OUTFILE}
echo ".moduleHeader {background: #EB6E08; font-size: bigger; font-weight: bold; text-align: center; width: 33em; margin-top: 1em; margin-bottom: 0em; }" >> ${OUTFILE}
echo ".moduleSummary {background: #FCBA82; font-weight: bold; width: 20em; float:left;}" >> ${OUTFILE}
echo ".moduleTotalExes {background: #FCBA82;font-weight: bold;  width: 3em; text-align: center; float:left;}" >> ${OUTFILE}
echo ".moduleTotalPasses {background: #FCBA82;font-weight: bold;  width: 3em; text-align: center; float:left;}" >> ${OUTFILE}
echo ".moduleTotalFails {background: #FCBA82;font-weight: bold;  width: 3em; text-align: center; float:left;}" >> ${OUTFILE}
echo ".moduleTotalSkipped {background: #FCBA82;font-weight: bold;  width: 3em; text-align: center; float:left;}" >> ${OUTFILE}

echo ".suiteSummary {background: #FF9946; font-weight: bold; width: 20em; float:left;}" >> ${OUTFILE}
echo ".suiteTotalExes {background: #FF9946;font-weight: bold;  width: 3em; text-align: center; float:left;}" >> ${OUTFILE}
echo ".suiteTotalPasses {background: #FF9946;font-weight: bold;  width: 3em; text-align: center; float:left;}" >> ${OUTFILE}
echo ".suiteTotalFails {background: #FF9946;font-weight: bold;  width: 3em; text-align: center; float:left;}" >> ${OUTFILE}
echo ".suiteTotalSkipped {background: #FF9946;font-weight: bold;  width: 3em; text-align: center; float:left;}" >> ${OUTFILE}

echo "</style>" >> ${OUTFILE}
echo "</head>" >> ${OUTFILE}
echo "<body>" >> ${OUTFILE}
echo "<div id="wrapper">" >> ${OUTFILE}
echo "<h1>QGIS Unit Tests</h1>" >> ${OUTFILE}
echo "<h2>`date +'%d %h %Y : %H h %M'`</h2>" >> ${OUTFILE}
for DIR in $DIRS
do
  echo "<div class='module'>"  >> ${OUTFILE}
  echo "<div class='moduleHeader'>Module : ${DIR}</div><br/>" >> ${OUTFILE}
  #print col headers
  echo "<div class="colSummary">&nbsp;</div>" >> ${OUTFILE}
  echo "<div class="colTotalExes">#</div>" >> ${OUTFILE}
  echo "<div class="colTotalPasses">P</div>" >> ${OUTFILE}
  echo "<div class="colTotalFails">F</div>" >> ${OUTFILE}
  echo "<div class="colTotalSkipped">S</div>" >> ${OUTFILE}
  echo "<br/>" >> ${OUTFILE}
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
    if (( $FAILED ))
    then
      echo "<div class="statusFail">&nbsp;</div>" >> ${OUTFILE}
    else
      echo "<div class="statusPass">&nbsp;</div>" >> ${OUTFILE}
    fi
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
  if (( $DIRFAILED ))
  then
    echo "<div class="statusFail">&nbsp;</div>" >> ${OUTFILE}
  else
    echo "<div class="statusPass">&nbsp;</div>" >> ${OUTFILE}
  fi
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
if (( $TOTALFAILED ))
then
  echo "<div class="statusFail">&nbsp;</div>" >> ${OUTFILE}
else
  echo "<div class="statusPass">&nbsp;</div>" >> ${OUTFILE}
fi
echo "<div class="suiteSummary">Totals:</div>" >> ${OUTFILE}
echo "<div class="suiteTotalExes">$TOTALEXES</div>" >> ${OUTFILE}
echo "<div class="suiteTotalPasses">$TOTALPASSED</div>" >> ${OUTFILE}
echo "<div class="suiteTotalFails">$TOTALFAILED</div>" >> ${OUTFILE}
echo "<div class="suiteTotalSkipped">$TOTALSKIPPED</div>" >> ${OUTFILE}
echo "<br/>" >> ${OUTFILE}
echo "</div> <!-- end of wrapper -->" >> ${OUTFILE}
echo "</body>" >> ${OUTFILE}
echo "</html>" >> ${OUTFILE}
firefox ${OUTFILE}
