#!/usr/bin/env bash
###########################################################################
#    chkcopyrights.sh
#    ---------------------
#    Date                 : October 2012
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


licensecheck -r . |
	grep -v -E "\/debian\/|\.\/python\/ext-libs|: BSD \(3 clause\)|: GPL \(v[23] or later\)$|: LGPL \(v2 or later\)$|: MIT\/X11 \(BSD like\)$|: Apache \(v2\.0\) GPL \(v2 or later\)$|: LGPL$|: Apache \(v2\.0\)$|: zlib\/libpng$|: GPL LGPL$|GENERATED FILE" |
	sed -e "s/:.*$//"
