#!/bin/bash
###########################################################################
#    pull_ts.sh
#    ---------------------
#    Date                 : November 2014
#    Copyright            : (C) 2014 by Tim Sutton
#    Email                : jef at norbit dot de
###########################################################################
#                                                                         #
#   This program is free software; you can redistribute it and/or modify  #
#   it under the terms of the GNU General Public License as published by  #
#   the Free Software Foundation; either version 2 of the License, or     #
#   (at your option) any later version.                                   #
#                                                                         #
###########################################################################

# Pull the translations from transifex and update TRANSLATORS

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

tx pull -a -s

echo Updating TRANSLATORS File
./scripts/tsstat.pl >doc/TRANSLATORS
