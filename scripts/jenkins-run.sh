#!/bin/bash

if [ -d build ]
then
    rm -rf build
fi

mkdir build
cd build
cmake ..
#xvfb-run --auto-servernum --server-num=1 --server-args="-screen 0 1024x768x24" make Experimental || true
make Experimental || true
#TRES=0
#ctest -T test --no-compress-output || true
if [ -f Testing/TAG ] ; then
   xsltproc ../tests/ctest2junix.xsl Testing/`head -n 1 < Testing/TAG`/Test.xml > CTestResults.xml
fi
