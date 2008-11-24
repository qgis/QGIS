#!/bin/sh
# Update the translation files with strings used in QGIS
# 1. create a clean Qt .pro file for the project
# 2. run lupdate using the .pro file from step 1
# 3. remove the .pro
# Note the .pro file must NOT be named qgis.pro as this
# name is reserved for the Windows qmake project file
# update_ts_files.sh,v 1.3 2004/07/14 18:16:24 gsherman Exp

PATH=$QTDIR/bin:$PATH

#first tar the qt_xx.ts files in i18n folder such that lupdate does not 
#merge the qgis strings to them
echo Creating qt_ts.tar
tar -cvf i18n/qt_ts.tar i18n/qt_*.ts
rm i18n/qt_*.ts
echo Creating qmake project file
qmake -project -o qgis_ts.pro
echo Updating translation files
lupdate -verbose qgis_ts.pro
echo Removing qmake project file
rm qgis_ts.pro
echo Unpacking qt_ts.tar
tar -xvf i18n/qt_ts.tar
