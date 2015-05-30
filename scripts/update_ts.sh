#!/bin/bash
###########################################################################
#    update_ts.sh
#    ---------------------
#    Date                 : November 2014
#    Copyright            : (C) 2014 by Juergen E. Fischer
#    Email                : jef at norbit dot de
###########################################################################
#                                                                         #
#   This program is free software; you can redistribute it and/or modify  #
#   it under the terms of the GNU General Public License as published by  #
#   the Free Software Foundation; either version 2 of the License, or     #
#   (at your option) any later version.                                   #
#                                                                         #
###########################################################################

set -e

case "$1" in
pull|push)
	;;

*)
	echo "usage: $(basename $0) {push|pull}"
	exit 1
esac

cleanup() {
	if [ -f i18n/backup.tar ]; then
		echo Restoring files...
		tar -xf i18n/backup.tar
	fi

	echo Removing temporary files
	for i in \
		python/python-i18n.{ts,cpp} \
		python/plugins/*/python-i18n.{ts,cpp} \
		src/plugins/grass/grasslabels-i18n.cpp \
		i18n/backup.tar \
		qgis_ts.pro
	do
		[ -f "$i" ] && rm "$i"
	done

	trap "" EXIT
}

PATH=$QTDIR/bin:$PATH

if type qmake-qt4 >/dev/null 2>&1; then
	QMAKE=qmake-qt4
else
	QMAKE=qmake
fi

if ! type pylupdate4 >/dev/null 2>&1; then
      echo "pylupdate4 not found"
      exit 1
fi

if type lupdate-qt4 >/dev/null 2>&1; then
	LUPDATE=lupdate-qt4
else
	LUPDATE=lupdate
fi

if ! type tx >/dev/null 2>&1; then
	echo "tx not found"
	exit 1
fi

trap cleanup EXIT

echo Saving translations
files="$(find python -name "*.ts") src/plugins/plugin_template/plugingui.cpp src/plugins/plugin_template/plugin.cpp"
[ $1 = push ] && files="$files i18n/qgis_*.ts"
tar --remove-files -cf i18n/backup.tar $files

if [ $1 = push ]; then
	echo Pulling source from transifex...
	tx pull -s -l none
else
	rm i18n/qgis_*.ts

	echo Pulling new translations...
	tx pull -a -s --minimum-perc=35
fi

echo Updating python translations
cd python
pylupdate4 user.py utils.py {console,pyplugin_installer}/*.{py,ui} -ts python-i18n.ts
perl ../scripts/ts2cpp.pl python-i18n.ts python-i18n.cpp
rm python-i18n.ts
cd ..
for i in python/plugins/*/CMakeLists.txt; do
	cd ${i%/*}
	pylupdate4 $(find . -name "*.py" -o -name "*.ui") -ts python-i18n.ts
	perl ../../../scripts/ts2cpp.pl python-i18n.ts python-i18n.cpp
	rm python-i18n.ts
	cd ../../..
done

echo Updating GRASS module translations
perl scripts/qgm2cpp.pl >src/plugins/grass/grasslabels-i18n.cpp

echo Creating qmake project file
$QMAKE -project -o qgis_ts.pro -nopwd src python i18n

echo Updating translations
$LUPDATE -locations absolute -verbose qgis_ts.pro

if [ $1 = push ]; then
	echo Pushing translation...
	tx push -s
else
	echo Updating TRANSLATORS File
	./scripts/tsstat.pl >doc/TRANSLATORS
fi
