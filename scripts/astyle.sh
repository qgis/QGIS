#!/bin/bash
###########################################################################
#    astyle.sh
#    ---------------------
#    Date                 : August 2008
#    Copyright            : (C) 2008 by Juergen E. Fischer
#    Email                : jef at norbit dot de
###########################################################################
#                                                                         #
#   This program is free software; you can redistribute it and/or modify  #
#   it under the terms of the GNU General Public License as published by  #
#   the Free Software Foundation; either version 2 of the License, or     #
#   (at your option) any later version.                                   #
#                                                                         #
###########################################################################

for ASTYLE in $(dirname $0)/qgisstyle $(dirname $0)/RelWithDebInfo/qgisstyle
do
	if type -p $ASTYLE >/dev/null; then
		break
	fi
	ASTYLE=
done

if [ -z "$ASTYLE" ]; then
	echo "qgisstyle not found - please enable WITH_ASTYLE in cmake and build it" >&2
	exit 1	
fi

if type -p tput >/dev/null; then
	elcr="$ASTYLEPROGRESS$(tput el)$(tput cr)"
else
	elcr="$ASTYLEPROGRESS                   \r"
fi

if ! type -p flip >/dev/null; then
	if type -p dos2unix >/dev/null; then
		flip() {
			dos2unix -k $2
		}
	else
		echo "flip not found" >&2
		flip() {
			:
		}
	fi
fi

if ! type -p autopep8 >/dev/null; then
	echo "autopep8 not found" >&2
	autopep8() {
		:
	}
fi

ASTYLEOPTS=$(dirname $0)/astyle.options
if type -p cygpath >/dev/null; then
	ASTYLEOPTS="$(cygpath -w $ASTYLEOPTS)"
fi

set -e

astyleit() {
	$ASTYLE --options="$ASTYLEOPTS" "$1"
	modified=$1.unify_includes_modified
	cp "$1" "$modified"
	scripts/unify_includes.pl "$modified"
	diff "$1" "$modified" >/dev/null || mv "$modified" "$1"
	rm -f "$modified"
}

for f in "$@"; do
	case "$f" in
		src/app/gps/qwtpolar-*|src/core/gps/qextserialport/*|src/plugins/grass/qtermwidget/*|src/astyle/*|python/ext-libs/*|src/providers/spatialite/qspatialite/*|src/plugins/dxf2shp_converter/dxflib/src/*|src/plugins/globe/osgEarthQt/*|src/plugins/globe/osgEarthUtil/*|python/ext-libs/*|*/ui_*.py)
			echo -ne "$f skipped $elcr"
			continue
			;;

		*.cpp|*.h|*.c|*.h|*.cxx|*.hxx|*.c++|*.h++|*.cc|*.hh|*.C|*.H|*.hpp)
			if [ -x "$f" ]; then
				chmod a-x "$f"
			fi
			cmd=astyleit
			;;

		*.ui|*.qgm|*.txt|*.t2t|resources/context_help/*)
			cmd=:
			;;

		*.py)
			#cmd="autopep8 --in-place --ignore=E111,E128,E201,E202,E203,E211,E221,E222,E225,E226,E227,E231,E241,E261,E265,E272,E302,E303,E501,E701"
			echo -ne "Formatting $f $elcr"
			cmd="autopep8 --in-place --ignore=E261,E265,E402,E501"
			;;

		*.sip)
			cmd="perl -i.prepare -pe 's/[\r\t ]+$//; s#^(\s*)/\*[*!]\s*([^\s*].*)\s*\$#\$1/** \u\$2\n#;'"
			;;

		*)
			echo -ne "$f skipped $elcr"
			continue
			;;
	esac

	if ! [ -f "$f" ]; then
		echo "$f not found" >&2
		continue
	fi

	if [[ -f $f && `head -c 3 $f` == $'\xef\xbb\xbf' ]]; then
		mv $f $f.bom
		tail -c +4 $f.bom > $f
		echo "removed BOM from $f"
	fi

	modified=$f.flip_modified
	cp "$f" "$modified"
	flip -ub "$modified"
	diff "$f" "$modified" >/dev/null || mv "$modified" "$f"
	rm -f "$modified"
	eval "$cmd '$f'"
done
