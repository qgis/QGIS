#!/bin/bash
###########################################################################
#    astyle-all.sh
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

set -e

export elcr="$(tput el)$(tput cr)"

find python src tests -type f -print | while read f; do
	case "$f" in
        src/app/gps/qwtpolar-*|src/core/spatialite/*|src/core/spatialindex/src/*|src/core/gps/qextserialport/*|src/plugins/grass/qtermwidget/*|src/astyle/*|python/pyspatialite/*|src/providers/sqlanywhere/sqlanyconnection/*|src/providers/spatialite/qspatialite/*|src/plugins/dxf2shp_converter/dxflib/src/*|src/plugins/globe/osgEarthQt/*|src/plugins/globe/osgEarthUtil/*)
                echo $f skipped
                continue
                ;;

        *.cpp|*.h|*.c|*.h|*.cxx|*.hxx|*.c++|*.h++|*.cc|*.hh|*.C|*.H|*.hpp)
                cmd=astyle.sh
                ;;

        *.ui|*.qgm|*.txt|*.t2t|*.sip|resources/context_help/*)
                cmd="flip -ub"
                ;;

	*.py)
                cmd="perl -i.prepare -pe 's/[\r\t ]+$//;'"
		;;

        *)
                echo -ne "$f skipped $elcr"
                continue
                ;;
        esac

        if [ -f "$f.astyle" ]; then
		# reformat backup
                cp "$f.astyle" "$f"
                touch -r "$f.astyle" "$f"
        else
		# make backup
                cp "$f" "$f.astyle"
                touch -r "$f" "$f.astyle"
        fi

  	echo -ne "Reformating $f $elcr"
	eval "$cmd '$f'"
done

echo
