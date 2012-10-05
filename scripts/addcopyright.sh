#!/bin/bash
###########################################################################
#    addcopyright.sh
#    ---------------------
#    Date                 : May 2012
#    Copyright            : (C) 2012 by Juergen E. Fischer
#    Email                : jef at norbit dot de
###########################################################################
#                                                                         #
#   This program is free software; you can redistribute it and/or modify  #
#   it under the terms of the GNU General Public License as published by  #
#   the Free Software Foundation; either version 2 of the License, or     #
#   (at your option) any later version.                                   #
#                                                                         #
###########################################################################

# licensecheck -r src

set -e

for i in $(<files); do
	echo $i >&2
	author=
	authordate=
	eval $(git log --reverse --pretty="export author='%an' authordate=\"\$(date --date='%ai' +'%%B %Y')\"" $i | head -1)
	basename=$(basename $i)
	authoryear=${authordate#* }

        case $i in
	# Override author if initial commit was by someone else
	python/plugins/sextante/*)
		author=volayaf
		;;

	src/app/gps/qwtpolar-*|src/app/qtmain_android.cpp|src/core/spatialite/*|src/core/spatialindex/src/*|src/core/gps/qextserialport/*|src/plugins/grass/qtermwidget/*|src/astyle/*|python/pyspatialite/*|src/providers/sqlanywhere/sqlanyconnection/*)
		# Skip third party files
                echo $f skipped
                continue
                ;;

	esac

	case $author in
	morb_au)
		authorname="Brendan Morley"
		authoremail="morb at ozemail dot com dot au"
		;;

	endmax)
		authorname="Massimo Endrighi"
		authoremail="massimo dot endrighi at geopartner dot it"
		;;

	rugginoso)
		authorname="Lorenzo Masini"
		authoremail="rugginoso at develer dot com"
		;;

	homann)
		authorname="Magnus Homann"
		authoremail="magnus at homann dot se"
		;;

	gsherman)
		authorname="Gary Sherman"
		authoremail="gsherman at geoapt dot com"
		;;

	kyngchaos)
		authorname="William Kyngesburye"
		authoremail="kyngchaos at kyngchaos dot com"
		;;

	volayaf)
		authorname="Victor Olaya"
		authoremail="volayaf at gmail dot com"
		;;

	jef|"Juergen E. Fischer")
		authorname="Juergen E. Fischer"
		authoremail="jef at norbit dot de"
		;;

	"Salvatore Larosa")
		authorname="Salvatore Larosa"
		authoremail="lrssvtml at gmail dot com"
		;;

	wonder|"Martin Dobias")
		authorname="Martin Dobias"
		authoremail="wonder.sk at gmail.com"
		;;

	"Marco Hugentobler"|mhugent)
		authorname="Marco Hugentobler"
		authoremail="marco dot hugentobler at sourcepole dot ch"
		;;

	"Pirmin Kalberer")
		authorname="Pirmin Kalberer"
		authoremail="pka at sourcepole dot ch"
		;;

        "Alexander Bruy")
		authorname="Alexander Bruy"
                authoremail="alexander dot bruy at gmail dot com"
		;;

	brushtyler|"Giuseppe Sucameli")
		authorname="Giuseppe Sucameli"
                authoremail="brush dot tyler at gmail dot com"
		;;

	"cfarmer")
		authorname="Carson J. Q. Farmer"
		authoremail="carson dot farmer at gmail dot com"
		;;

	rblazek|"Radim Blazek")
		authorname="Radim Blazek"
		authoremail="radim dot blazek at gmail dot com"
		;;

	timlinux|"Tim Sutton")
		authorname="Tim Sutton"
		authoremail="tim dot linfiniti at com"
		;;

	*)
		echo "Author $author not found."
		exit 1
		;;
	esac

	origsrc=$i
	if [ -f "$i.nocopyright" ]; then
		origsrc=$i.nocopyright
	fi

	src=$origsrc
	dst=$src.new

	shebang=$(head -1 $src)
	case "$shebang" in
	'#!'*)
		shebang="$shebang
"
		src="<(tail -n +2 $src)"
		;;
	*)
		shebang=
		;;
	esac

	case "$origsrc" in
	*.sh)
		eval cat - $src >$dst <<EOF
$shebang###########################################################################
#    $basename
#    ---------------------
#    Date                 : $authordate
#    Copyright            : (C) $authoryear by $authorname
#    Email                : $authoremail
###########################################################################
#                                                                         #
#   This program is free software; you can redistribute it and/or modify  #
#   it under the terms of the GNU General Public License as published by  #
#   the Free Software Foundation; either version 2 of the License, or     #
#   (at your option) any later version.                                   #
#                                                                         #
###########################################################################

EOF
		;;
	*.py)
		cat - $src >$dst <<EOF
$shebang# -*- coding: utf-8 -*-

"""
***************************************************************************
    $basename
    ---------------------
    Date                 : $authordate
    Copyright            : (C) $authoryear by $authorname
    Email                : $authoremail
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = '$authorname'
__date__ = '$authordate'
__copyright__ = '(C) $authoryear, $authorname'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '\$Format:%H$'

EOF
		;;

	*.cpp|*.h)
		cat - $src >$dst <<EOF
/***************************************************************************
    $basename
    ---------------------
    begin                : $authordate
    copyright            : (C) $authoryear by $authorname
    email                : $authoremail
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
EOF
		;;

	*)
		echo "$i skipped"
		continue
	esac

	[ -f $i.nocopyright ] || mv $i $i.nocopyright
	cp $dst $origsrc
done
