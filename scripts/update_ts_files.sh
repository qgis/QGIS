#!/bin/bash
###########################################################################
#    update_ts_files.sh
#    ---------------------
#    Date                 : July 2007
#    Copyright            : (C) 2007 by Tim Sutton
#    Email                : tim dot linfiniti at com
###########################################################################
#                                                                         #
#   This program is free software; you can redistribute it and/or modify  #
#   it under the terms of the GNU General Public License as published by  #
#   the Free Software Foundation; either version 2 of the License, or     #
#   (at your option) any later version.                                   #
#                                                                         #
###########################################################################

# Update the translation files with strings used in QGIS
# 1. create a clean Qt .pro file for the project
# 2. run lupdate using the .pro file from step 1
# 3. remove the .pro
# Note the .pro file must NOT be named qgis.pro as this
# name is reserved for the Windows qmake project file

set -e

cleanup() {
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

trap cleanup EXIT

PATH=$QTDIR/bin:$PATH

if type qmake-qt4 >/dev/null 2>&1; then
	QMAKE=qmake-qt4
else
	QMAKE=qmake
fi

if type lupdate-qt4 >/dev/null 2>&1; then
	LUPDATE=lupdate-qt4
else
	LUPDATE=lupdate
fi

exclude=
opts=
while (( $# > 0 )); do
  arg=$1
  shift

  if [ "$arg" = "-a" ]; then
    arg=$1
    shift
    if [ -f "i18n/qgis_$arg.ts" ]; then
      echo "cannot add existing translation $arg"
      exit 1
    else
      add="$add $arg"
    fi
  elif [ -f "i18n/qgis_$arg.ts" ]; then
    exclude="$exclude --exclude i18n/qgis_$arg.ts"
  else
    opts="$opts $arg"
  fi
done

if [ -n "$exclude" -o -n "$add" ]; then
  echo Saving excluded translations
  tar --remove-files -cf i18n/qgis_ts.tar i18n/qgis_*.ts$exclude
fi
echo Updating python translations
cd python
pylupdate4 console/console.py \
console/console_sci.py \
console/console_editor.py \
console/console_output.py utils.py -ts python-i18n.ts
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
if [ -n "$add" ]; then
	for i in $add; do
		echo "Adding translation for $i"
		echo "TRANSLATIONS += i18n/qgis_$i.ts" >> qgis_ts.pro
	done
fi
echo Updating translations
$LUPDATE$opts -verbose qgis_ts.pro

cleanup

echo Updating TRANSLATORS File
./scripts/tsstat.pl > doc/TRANSLATORS

if [ -n "$add" ]; then
	for i in $add; do
		if [ -f i18n/qgis_$i.ts ]; then
			git add i18n/qgis_$i.ts
		else
			echo "Translation for $i was not added"
			exit 1
		fi
	done
fi
