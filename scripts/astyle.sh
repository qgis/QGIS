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

if ! type -p flip >/dev/null; then
	echo "flip not found" >&2
	flip() {
		:
	}
fi

if ! type -p pep8 >/dev/null; then
	echo "pep8 not found" >&2
	pep8() {
		:
	}
fi

set -e

export ARTISTIC_STYLE_OPTIONS="\
--preserve-date \
--indent-preprocessor \
--brackets=break \
--convert-tabs \
--indent=spaces=2 \
--indent-classes \
--indent-labels \
--indent-namespaces \
--indent-switches \
--one-line=keep-blocks \
--one-line=keep-statements \
--max-instatement-indent=40 \
--min-conditional-indent=-1 \
--suffix=none"

export ARTISTIC_STYLE_OPTIONS="\
$ARTISTIC_STYLE_OPTIONS \
--pad=oper \
--pad=paren-in \
--unpad=paren"

for f in "$@"; do
	case "$f" in
        src/app/gps/qwtpolar-*|src/core/gps/qextserialport/*|src/plugins/grass/qtermwidget/*|src/astyle/*|python/ext-libs/*|src/providers/spatialite/qspatialite/*|src/plugins/dxf2shp_converter/dxflib/src/*|src/plugins/globe/osgEarthQt/*|src/plugins/globe/osgEarthUtil/*|python/ext-libs/*|*/ui_*.py)
                echo -ne "$f skipped $elcr"
                continue
                ;;

        *.cpp|*.h|*.c|*.h|*.cxx|*.hxx|*.c++|*.h++|*.cc|*.hh|*.C|*.H|*.hpp)
                cmd="$ASTYLE $ARTISTIC_STYLE_OPTIONS"
                ;;

        *.ui|*.qgm|*.txt|*.t2t|resources/context_help/*)
                cmd=:
                ;;

	*.py)
		cmd="pep8 --ignore=E111,E128,E201,E202,E203,E211,E221,E222,E225,E226,E227,E231,E241,E261,E265,E272,E302,E303,E501,E701"
		;;

        *.sip)
                cmd="perl -i.prepare -pe 's/[\r\t ]+$//;'"
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

	flip -ub "$f" 
	#qgsloggermig.pl "$f"
        eval "$cmd '$f'"
done
