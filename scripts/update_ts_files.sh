#!/bin/bash
# Update the translation files with strings used in QGIS
# 1. create a clean Qt .pro file for the project
# 2. run lupdate using the .pro file from step 1
# 3. remove the .pro
# Note the .pro file must NOT be named qgis.pro as this
# name is reserved for the Windows qmake project file
# update_ts_files.sh,v 1.3 2004/07/14 18:16:24 gsherman Exp

set -e

PATH=$QTDIR/bin:$PATH

#first tar the qt_xx.ts files in i18n folder such that lupdate does not 
#merge the qgis strings to them
echo Creating qt_ts.tar
tar --remove-files -cvf i18n/qt_ts.tar i18n/qt_*.ts
exclude=
for i in "$@"; do
  exclude="$exclude --exclude i18n/qgis_$i.ts"
done
if [ -n "$exclude" ]; then
  tar --remove-files -cvf i18n/qgis_ts.tar i18n/qgis_*.ts$exclude
fi
echo Updating python plugin translations
for i in python/plugins/*/.; do
	cd $i
	pylupdate4 $(find . -name "*.py") -ts python-i18n.ts
	perl ../../../scripts/ts2cpp.pl python-i18n.ts python-i18n.cpp
	rm python-i18n.ts
	cd ../../..
done
echo Creating qmake project file
qmake -project -o qgis_ts.pro -nopwd src python i18n
echo Updating translation files
lupdate -verbose qgis_ts.pro
echo Removing temporary python plugin translation files
perl -i.bak -ne 'print unless /^\s+<location.*python-i18n\.cpp.*$/;' i18n/qgis_*.ts
rm python/plugins/*/python-i18n.cpp i18n/qgis_*.ts.bak
echo Removing qmake project file
rm qgis_ts.pro
echo Unpacking qt_ts.tar
tar -xvf i18n/qt_ts.tar 
rm i18n/qt_ts.tar
if [ -f i18n/qgis_ts.tar ]; then
  echo Unpacking i18n/qgis_ts.tar
  tar -xvf i18n/qgis_ts.tar 
  rm i18n/qgis_ts.tar
fi
