#!/bin/bash
for MY_FILENAME in $(ls *.png); do 
    FILENAMEBASE=`echo $MY_FILENAME |sed 's/\.png//g'`
    convert $MY_FILENAME $FILENAMEBASE.eps  
done
