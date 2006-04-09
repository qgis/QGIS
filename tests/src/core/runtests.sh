#!/bin/bash
LIST=`ls -lah |grep rwxr-xr-x |grep -v ^d |grep -v pl$ |grep -v ~$ |grep -v .sh$ |awk '{print $8}'|awk '$1=$1' RS=`
for FILE in $LIST; 
do 
  echo "Running $FILE"
  `./${FILE}`
done
