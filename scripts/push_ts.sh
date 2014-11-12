#!/bin/bash
###########################################################################
#    push_ts.sh
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

# Update the english translation and push it to transifex

set -e

cleanup() {
	if [ -f i18n/python_ts.tar ]; then
		tar -xf i18n/python_ts.tar
	fi
	if [ -f i18n/qgis_ts.tar ]; then
		echo Restoring excluded translations
		tar -xf i18n/qgis_ts.tar
	fi

	echo Removing temporary files
	perl -i.bak -ne 'print unless /^\s+<location.*python-i18n\.cpp.*$/;' i18n/qgis_*.ts
	for i in \
		python/python-i18n.{ts,cpp} \
		python/plugins/*/python-i18n.{ts,cpp} \
		i18n/qgis_*.ts.bak \
		src/plugins/grass/grasslabels-i18n.cpp \
		i18n/qgis_ts.tar \
		i18n/python_ts.tar \
		qgis_ts.pro
	do
		[ -f "$i" ] && rm "$i"
	done

	for i in \
		src/plugins/plugin_template/plugingui.cpp \
		src/plugins/plugin_template/plugin.cpp
	do
		[ -f "$i.save" ] && mv "$i.save" "$i"
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

if ! type tx >/dev/null 2>&1; then
      echo "tx not found"
      exit 1
fi

if type lupdate-qt4 >/dev/null 2>&1; then
	LUPDATE=lupdate-qt4
else
	LUPDATE=lupdate
fi

trap cleanup EXIT

tar --remove-files -cf i18n/python_ts.tar $(find python -name "*.ts")
echo Saving translations
tar --remove-files -cf i18n/qgis_ts.tar i18n/qgis_*.ts

echo Pulling source from transifex...
tx pull -s

echo Updating python translations
cd python
pylupdate4 utils.py {console,pyplugin_installer}/*.{py,ui} -ts python-i18n.ts
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
mv src/plugins/plugin_template/plugingui.cpp src/plugins/plugin_template/plugingui.cpp.save
echo Creating qmake project file
for i in \
	src/plugins/plugin_template/plugingui.cpp \
	src/plugins/plugin_template/plugin.cpp
do
	[ -f "$i" ] && mv "$i" "$i.save"
done
$QMAKE -project -o qgis_ts.pro -nopwd src python i18n

echo Updating translations
$LUPDATE -locations none -verbose qgis_ts.pro

tx push -s
