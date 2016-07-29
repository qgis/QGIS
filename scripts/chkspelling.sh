#!/bin/bash
###########################################################################
#    chkspelling.sh
#    ---------------------
#    Date                 : December 2009
#    Copyright            : (C) 2009 by Juergen E. Fischer
#    Email                : jef at norbit dot de
###########################################################################
#                                                                         #
#   This program is free software; you can redistribute it and/or modify  #
#   it under the terms of the GNU General Public License as published by  #
#   the Free Software Foundation; either version 2 of the License, or     #
#   (at your option) any later version.                                   #
#                                                                         #
###########################################################################



RE=$(echo $(cut -d: -f1 scripts/spelling.dat | sed -e 's/^/\\</; s/$/\\>|/;') | sed -e 's/| /|/g; s/|$//;')
EX="\.(svn-base|tmp|xpm|ts|o)|spelling\.dat|Exception_to_GPL_for_Qt.txt|sqlite3.c|qgisstyle|LexerR.py|debian/build*|ms-windows/osgeo4w|ChangeLog|src/plugins/grass/qtermwidget|src/app/gps/qwtpolar-|debian/tmp|src/plugins/dxf2shp_converter/dxflib|python/ext-libs|i18n/"

egrep --exclude=*.{png,svg,db,bz2,pdf,qgs,qml,api,pyc} --exclude-dir=.git --exclude-dir=debian/build* --color=always "$RE" -ir .  | egrep -iv "$EX"
