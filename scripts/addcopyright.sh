#!/usr/bin/env bash
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

export LC_TIME=C

set -e

if [ $# -gt 0 ]; then
	FILES="$@"
elif [ -f files ]; then
	FILES=$(<files)
else
	echo no files
	exit 1
fi

for i in $FILES; do
	echo "${i}" >&2
	author=
	authordate=
	eval $(git log --reverse --pretty="export author='%an' authordate=\"\$(date --date='%ai' +'%%B %Y')\"" $i | head -1)
	basename=$(basename "${i}")
	authoryear=${authordate#* }

        case $i in
	# Override author if initial commit was by someone else
	python/plugins/processing/*)
		author=volayaf
		;;

	src/app/qtmain_android.cpp)
		# Skip third party files
                echo "${i} skipped"
                continue
                ;;

	esac

	case "${author}" in
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
		authoremail="wonder dot sk at gmail dot com"
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

	pcav)
		authorname="Paolo Cavallini"
                authoremail="cavallini at faunalia dot it"
		;;

	"cfarmer")
		authorname="Carson J. Q. Farmer"
		authoremail="carson dot farmer at gmail dot com"
		;;

	rblazek|"Radim Blazek")
		authorname="Radim Blazek"
		authoremail="radim dot blazek at gmail dot com"
		;;

	marcopx)
		authorname="Marco Pasetti"
		authoremail="marco dot pasetti at alice dot it"
		;;

	timlinux|"Tim Sutton")
		authorname="Tim Sutton"
		authoremail="tim at linfiniti dot com"
		;;

	"Matthias Kuhn")
		authorname="Matthias Kuhn"
		authoremail="matthias at opengis dot ch"
		;;

	"Emilio Loi")
		authorname="Emilio Loi"
		authoremail="loi at faunalia dot it"
		;;

	"Nathan Woodrow"|Nathan)
		authorname="Nathan Woodrow"
		authoremail="woodrow dot nathan at gmail dot com"
		;;

	"Sandro Mani")
		authorname="Sandro Mani"
		authoremail="smani at sourcepole dot ch"
		;;

	"Chris Crook")
		authorname="Chris Crook"
		authoremail="ccrook at linz dot govt dot nz"
		;;

	"Hugo Mercier")
		authorname="Hugo Mercier"
		authoremail="hugo dot mercier at oslandia dot com"
		;;

	"Larry Shaffer")
		authorname="Larry Shaffer"
		authoremail="larrys at dakotacarto dot com"
		;;

	"Victor Olaya"|volaya)
		authorname="Victor Olaya"
		authoremail="volayaf at gmail dot com"
		;;

	elpaso)
		authorname="Alessandro Pasotti"
		authoremail="elpaso at itopen dot it"
		;;

        "Patrick Valsecchi")
		authorname="Patrick Valsecchi"
		authoremail="patrick dot valsecchi at camptocamp dot com"
		;;

	"Stéphane Brunner")
		authorname="Stéphane Brunner"
		authoremail="stephane dot brunner at camptocamp dot com"
		;;

	"ersts")
		authorname="Peter Ersts"
		authoremail="ersts at amnh dot org"
		;;

	"Etienne Tourigny")
		authorname="Etienne Tourigny"
		authoremail="etourigny dot dev at gmail dot com"
		;;

	"Nyall Dawson")
		authorname="Nyall Dawson"
		authoremail="nyall dot dawson at gmail dot com"
		;;

	"David")
		authorname="David Signer"
		authoremail="david at opengis dot ch"
		;;

	"Etienne Trimaille")
		authorname="Etienne Trimaille"
		authoremail="etienne dot trimaille at gmail dot com"
		;;

	"David Marteau")
		authorname="David Marteau"
		authoremail="david at innophi dot com"
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
		eval cat - $src >$dst <<EOF
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

	*.pl)
		eval cat - $src >$dst <<EOF
$shebang###########################################################################
#    $basename
#    ---------------------
#    begin                : $authordate
#    copyright            : (C) $authoryear by $authorname
#    email                : $authoremail
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

	*.bat|*.cmd)
		cat - $src >$dst <<EOF
REM ***************************************************************************
REM    $basename
REM    ---------------------
REM    begin                : $authordate
REM    copyright            : (C) $authoryear by $authorname
REM    email                : $authoremail
REM ***************************************************************************
REM *                                                                         *
REM *   This program is free software; you can redistribute it and/or modify  *
REM *   it under the terms of the GNU General Public License as published by  *
REM *   the Free Software Foundation; either version 2 of the License, or     *
REM *   (at your option) any later version.                                   *
REM *                                                                         *
REM ***************************************************************************
EOF
		;;

	*)
		echo "$i skipped"
		continue
	esac

	[ -f $i.nocopyright ] || mv $i $i.nocopyright
	cp $dst $origsrc
done
