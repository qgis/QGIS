#!/usr/bin/env bash
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

action=$1

case "$action" in
pull|push|update)
	;;

*)
	echo "usage: $(basename $0) {pull|{push|update} builddirectory [lang...]}"
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
		python/plugins/processing/processing-i18n.{ts,cpp} \
		src/plugins/grass/grasslabels-i18n.cpp \
		src/app/appinfo-i18n.cpp \
		i18n/backup.tar \
		qgis_ts.pro
	do
		[ -f "$i" ] && rm "$i"
	done

	trap "" EXIT
}

export QT_SELECT=5

PATH=$QTDIR/bin:$PATH

if type qmake-qt5 >/dev/null 2>&1; then
	QMAKE=qmake-qt5
else
	QMAKE=qmake
fi

if ! type pylupdate5 >/dev/null 2>&1; then
      echo "pylupdate5 not found"
      exit 1
fi

if type lupdate-qt5 >/dev/null 2>&1; then
	LUPDATE=lupdate-qt5
else
	LUPDATE=lupdate
fi

if ! type tx >/dev/null 2>&1; then
	echo "tx not found"
	exit 1
fi

files=
if [ -d "$2" ]; then
	builddir=$(realpath $2)
	textcpp=
	for i in $builddir/src/core/qgsexpression_texts.cpp; do
		if [ -f $i ]; then
			textcpp="$textcpp $i"
		elif [ "$action" != "pull" ]; then
			echo Generated help file $i not found
			exit 1
		fi
	done
	shift
	shift
	if [[ $# -gt 0 ]]; then
		for t in i18n/qgis_*.ts; do
			for l in "$@"; do
				if [ "i18n/qgis_$l.ts" = "$t" ]; then
					continue 2
				fi
			done
			files="$files $t"
		done
	fi

elif [ "$action" != "pull" ]; then
	echo Build directory not found
	exit 1
else
	shift
fi

trap cleanup EXIT

echo Saving translations
files="$files $(find python -name "*.ts")"
[ $action = push ] && files="$files i18n/qgis_*.ts"
[ -n "${files## }" ] && tar --remove-files -cf i18n/backup.tar $files

if [ $action = push ]; then
	echo Pulling source from transifex...
	tx pull -s -l none
	if ! [ -f "i18n/qgis_en.ts" ]; then
		echo Download of source translation failed
		exit 1
	fi
	cp i18n/qgis_en.ts /tmp/qgis_en.ts-downloaded
elif [ $action = pull ]; then
	rm i18n/qgis_*.ts

	echo Pulling new translations...
	if [ "$#" -gt 0 ]; then
		o=$*
		o="-l ${o// /,}"
	else
		o="-a"
	fi
	tx pull $o -s --minimum-perc=35
fi

echo Updating python translations
(
	cd python
	pylupdate5 user.py utils.py {console,pyplugin_installer}/*.{py,ui} -ts python-i18n.ts
	perl ../scripts/ts2cpp.pl python-i18n.ts python-i18n.cpp
	rm python-i18n.ts
)
for i in python/plugins/*/CMakeLists.txt; do
	cd ${i%/*}
	cat <<EOF >python-i18n.pro
SOURCES = $(find . -type f -name "*.py" -printf "	%p \
")

FORMS = $(find . -type f -name "*.ui" -printf "	%p \
")

TRANSLATIONS = python-i18n.ts
EOF

	pylupdate5 -tr-function trAlgorithm python-i18n.pro
	perl ../../../scripts/ts2cpp.pl python-i18n.ts python-i18n.cpp
	rm python-i18n.ts python-i18n.pro
	cd ../../..
done

echo Updating GRASS module translations
perl scripts/qgm2cpp.pl >src/plugins/grass/grasslabels-i18n.cpp

echo Updating processing translations
perl scripts/processing2cpp.pl python/plugins/processing/processing-i18n.cpp

echo Updating appinfo files
python scripts/appinfo2cpp.py >src/app/appinfo-i18n.cpp

echo Creating qmake project file
$QMAKE -project -o qgis_ts.pro -nopwd $PWD/src $PWD/python $PWD/i18n $textcpp

echo "TR_EXCLUDE = $(qmake -query QT_INSTALL_HEADERS)/*" >>qgis_ts.pro

echo Updating translations
$LUPDATE -locations absolute -verbose qgis_ts.pro

perl -i.bak -ne 'print unless /^\s+<location.*qgs(expression|contexthelp)_texts\.cpp.*$/;' i18n/qgis_*.ts

if [ $action = push ]; then
	cp i18n/qgis_en.ts /tmp/qgis_en.ts-uploading
	echo Pushing translation...
	fail=1
	for i in $(seq 10); do
		tx push -s && fail=0 && break
		echo Retrying...
		sleep 10
	done
	if (( fail )); then
		echo "Could not push translations"
		exit 1
	fi
else
	echo Updating TRANSLATORS File
	./scripts/tsstat.pl >doc/TRANSLATORS
fi
