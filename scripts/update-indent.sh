#!/bin/bash
###########################################################################
#    update-indent.sh
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


PATH=$PATH:$(dirname $0)

if ! type -p colordiff >/dev/null; then
	colordiff() {
		cat "$@"
	}
fi

if ! type -p flip >/dev/null; then
	echo flip not found
	exit 1
fi

set -e

# determine last commit
if [ -f .lastcommit ]; then
	REV0=$(<.lastcommit)
	svn revert -R .
	find . -name "*.prepare" -delete
	svn update -r$REV0
else
	REV0=$(svn info | sed -ne "s/Revision: //p")
fi

# update
MODIFIED=$(svn update | sed -ne "s/^[^ ]. *//p")
REV1=$(svn info | sed -ne "s/Revision: //p")

if [ "$REV0" -eq "$REV1" ]; then
	echo "No activity since last run."
	exit 0
fi

echo "Checking changes between $REV0 and $REV1"

ASTYLEDIFF=astyle.r$REV0-r$REV1.diff
>$ASTYLEDIFF

# reformat
for f in $MODIFIED; do
	case "$f" in
	src/core/spatialite/*|src/core/gps/qextserialport/*|src/plugins/grass/qtermwidget/*|src/astyle/*|python/pyspatialite/*)
		echo $f skipped
		continue
		;;

        *.cpp|*.h|*.c|*.h|*.cxx|*.hxx|*.c++|*.h++|*.cc|*.hh|*.C|*.H|*.hpp)
		cmd=astyle.sh
                ;;

	*.ui|*.qgm|*.txt|*.t2t|*.py|*.sip|resources/context_help/*)
		cmd="flip -ub"
		;;

        *)
		echo $f skipped
		continue
                ;;
        esac

	if ! [ -s $f ]; then
		# deleted
	 	continue
	fi

        m=$f.r$REV1.prepare

	cp $f $m
	$cmd $f
	if diff -u $m $f >>$ASTYLEDIFF; then
		# no difference found
		rm $m
	fi
done

if [ -s "$ASTYLEDIFF" ]; then
	if tty -s; then
		# review astyle changes
		colordiff <$ASTYLEDIFF | less -r
	else
		echo "Files changed (see $ASTYLEDIFF)"
	fi

	# just echo for now
	echo "svn commit -m \"automatic indentation update (r$REV0-r$REV1)\""
	[ -f .lastcommit ] && mv .lastcommit .prevcommit
	echo $REV1 >.lastcommit
else
	echo "No indentation updates."
	rm $ASTYLEDIFF
fi
