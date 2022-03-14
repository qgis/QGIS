#!/usr/bin/env bash
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

# sort by version option
SV=V
if [[ "$OSTYPE" =~ darwin* ]]; then
	SV=n
fi

min_version="3"
astyle_version_check() {
	[ $(printf "$($1 --version 2>&1 | cut -d ' ' -f4)\\n$min_version" | sort -${SV} | head -n1) = "$min_version" ]
}

for ASTYLE in ${QGISSTYLE} $(dirname "$0")/qgisstyle $(dirname "$0")/RelWithDebInfo/qgisstyle astyle
do
	if type -p "$ASTYLE" >/dev/null; then
		if astyle_version_check "$ASTYLE"; then
			break
		fi
	fi
	ASTYLE=
done

if [ -z "$ASTYLE" ]; then
	echo "qgisstyle / astyle not found - please install astyle >= $min_version or enable WITH_ASTYLE in cmake and build" >&2
	exit 1
fi

if type -p tput >/dev/null; then
	elcr="$ASTYLEPROGRESS$(tput el)$(tput cr)"
else
	elcr="$ASTYLEPROGRESS                   \\r"
fi

if ! type -p flip >/dev/null; then
	if type -p dos2unix >/dev/null; then
		flip() {
			dos2unix -q -k "$2"
		}
	else
    echo "flip not found" >&2
    echo "Try:"
    if [[ -f /etc/fedora-release ]]; then
      echo "  dnf install dos2unix";
    elif [[ -f /etc/debian_version ]]; then
      echo "  apt install flip";
    else
      echo "  installing flip or dos2unix from your package manager";
    fi
		flip() {
			:
		}
	fi
fi

if ! type -p autopep8 >/dev/null; then
	echo "autopep8 not found" >&2
	echo "Try:"
	if [[ -f /etc/fedora-release ]]; then
		echo "  dnf install python3-autopep8";
	elif [[ -f /etc/debian_version ]]; then
		echo "  apt install python3-autopep8";
	else
		echo "  installing python3-autopep8 from your package manager";
	fi
	autopep8() {
		:
	}
fi

ASTYLEOPTS=$(dirname "$0")/astyle.options
if type -p cygpath >/dev/null; then
	ASTYLEOPTS="$(cygpath -w "$ASTYLEOPTS")"
fi

if type -p wslpath >/dev/null; then
	ASTYLEOPTS="$(wslpath -a -w "$ASTYLEOPTS")"
fi

set -e

astyleit() {
	$ASTYLE --options="$ASTYLEOPTS" "$1"
        modified=$1.unify_includes_modified
	cp "$1" "$modified"
	perl -i.sortinc -n scripts/unify_includes.pl "$modified"
	scripts/doxygen_space.pl "$modified"
	diff "$1" "$modified" >/dev/null || mv "$modified" "$1"
	rm -f "$modified"
}

for f in "$@"; do
	case "$f" in
                src/plugins/grass/qtermwidget/*|external/libdxfrw/*|external/untwine/*|external/qwt*|external/o2/*|external/odbccpp/*|external/qt-unix-signals/*|external/rtree/*|external/astyle/*|external/kdbush/*|external/poly2tri/*|external/wintoast/*|external/qt3dextra-headers/*|external/lazperf/*|external/meshOptimizer/*|external/mapbox-vector-tile/*|python/ext-libs/*|ui_*.py|*.astyle|tests/testdata/*|editors/*)
			echo -ne "$f skipped $elcr"
			continue
			;;

		*.cpp|*.h|*.c|*.cxx|*.hxx|*.c++|*.h++|*.cc|*.hh|*.C|*.H|*.hpp|*.mm)
			if [ -x "$f" ]; then
				chmod a-x "$f"
			fi
			cmd=astyleit
			;;

		*.ui|*.qgm|*.txt)
			cmd=:
			;;

		*.py)
			#cmd="autopep8 --in-place --ignore=E111,E128,E201,E202,E203,E211,E221,E222,E225,E226,E227,E231,E241,E261,E265,E272,E302,E303,E501,E701"
			echo -ne "Formatting $f $elcr"
			cmd="autopep8 --in-place --ignore=E261,E265,E402,E501"
			;;

		*.sip)
			cmd="perl -i.prepare -pe 's/[\\r\\t ]+$//; s#^(\\s*)/\\*[*!]\\s*([^\\s*].*)\\s*\$#\$1/** \\u\$2\\n#;'"
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

	if [[ -f $f && $(head -c 3 "$f") == $'\xef\xbb\xbf' ]]; then
		mv "$f" "$f".bom
		tail -c +4 "$f".bom > "$f"
		echo "removed BOM from $f"
	fi

	modified=$f.flip_modified
	cp "$f" "$modified"
	flip -ub "$modified"
	diff "$f" "$modified" >/dev/null || mv "$modified" "$f"
	rm -f "$modified"
	eval "$cmd '$f'"
done
