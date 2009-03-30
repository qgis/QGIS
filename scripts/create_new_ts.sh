#!/bin/sh

if [ -z "$1" ] ; then
  echo 
  echo "Usage : $(basename $0) [Locale]"
  echo "Examples :"
  echo "$(basename $0) en_GB"
  echo 
  exit 0
fi

# Create a new .ts file for a new translation in qgis 
# 1. create a clean Qt .pro file for the project
# 2. run lupdate using the .pro file from step 1
# 3. remove the .pro
# update_ts_files.sh,v 1.3 2004/07/14 18:16:24 gsherman Exp

#first tar the qt_xx.ts files in i18n folder such that lupdate does not 
#merge the qgis strings to them
QTDIR=/usr/
echo Creating qt_ts.tar
tar -cvf i18n/qt_ts.tar i18n/qt_*.ts
rm i18n/qt_*.ts
echo Creating qmake project file
$QTDIR/bin/qmake -project -o qgis_ts.pro
#add our new translation to the pro file
echo "Creating new translation entry for $1 in the pro file"
echo "TRANSLATIONS += i18n/qgis_${1}.ts" >> qgis_ts.pro
echo Updating translation files
$QTDIR/bin/lupdate -verbose qgis_ts.pro
echo Removing qmake project file
rm qgis_ts.pro
echo Unpacking qt_ts.tar
tar -xvf i18n/qt_ts.tar
