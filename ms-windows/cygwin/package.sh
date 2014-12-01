#!/bin/bash

set -e

[ -d build ] || mkdir build
cd build

cmake -D BUILDNAME="cygwin" \
        -D SITE="qgis.org" \
        -D PEDANTIC=TRUE \
	-D WITH_INTERNAL_SPATIALITE=TRUE \
        -D WITH_GRASS=FALSE \
        -D WITH_SPATIALITE=TRUE \
        -D WITH_QSPATIALITE=TRUE \
        -D WITH_SERVER=TRUE \
        -D WITH_ASTYLE=TRUE \
        -D WITH_GLOBE=TRUE \
        -D WITH_TOUCH=TRUE \
        -D WITH_ORACLE=FALSE \
	-D CMAKE_LEGACY_CYGWIN_WIN32=0 \
	-D PYUIC4_PROGRAM=/usr/lib/python2.7/site-packages/PyQt4/pyuic4 \
	-D PYRCC4_PROGRAM=/usr/lib/python2.7/site-packages/PyQt4/pyrcc4.exe \
	-D WITH_GLOBE=NO \
	-D ENABLE_TESTS=YES \
	-D CMAKE_INSTALL_PREFIX=/usr \
	-D WITH_CUSTOM_WIDGETS=TRUE \
        ../../..

make -j8
make test
make install
