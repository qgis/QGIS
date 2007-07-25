#!/bin/sh
# Update the translation files with strings used in QGIS
# 1. create a clean Qt .pro file for the project
# 2. run lupdate using the .pro file from step 1
# 3. remove the .pro
# Note the .pro file must NOT be named qgis.pro as this
# name is reserved for the Windows qmake project file
# update_ts_files.sh,v 1.3 2004/07/14 18:16:24 gsherman Exp

echo Creating qmake project file
$QTDIR/bin/qmake -project -o qgis_ts.pro
echo Updating translation files
$QTDIR/bin/lupdate -verbose qgis_ts.pro
echo Removing qmake project file
rm qgis_ts.pro
