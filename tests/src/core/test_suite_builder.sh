#!/bin/bash
LIST=`ls ../../../src/core/ |grep .cpp |grep ^qgs |grep -v ~$ |grep -v moc.cpp$ | sed 's/.cpp//g' |awk '$1=$1' RS= |sort`
for FILE in $LIST
do 
  CLASSNAME=`grep -o "::Qgs[A-Za-z0-9]*(" ../../../src/core/${FILE}.cpp |head -1 | sed 's/:://g'| sed 's/(//g'`
  ./test_builder.pl $CLASSNAME
  #svn add test${FILE}.cpp
done
make install
