#!/bin/bash
# licensecheck -r src

for i in $(<files); do
	author=
	authordate=
	eval $(git log --reverse --pretty="export author='%an' authordate=\"\$(date --date='%ai' +'%%B %Y')\"" $i | head -1)
	basename=$(basename $i)
	authoryear=${authordate#* }
	case $author in
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

	rblazek)
		authorname="Radim Blazek"
		authoremail="radim dot blazek at gmail dot com"
		;;

	timlinux)
		authorname="Tim Sutton"
		authoremail="tim dot linfiniti at com"
		;;

	*)
		echo "Author $author not found."
		exit 1
		;;
	esac

	src=$i
	if [ -f "$i.nocopyright" ]; then
		src=$i.nocopyright
	fi

	cat - $src >/tmp/new <<EOF
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
	[ -f $i.nocopyright ] || mv $i $i.nocopyright
	cp /tmp/new $i
done
