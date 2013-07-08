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
EX="\.(svn-base|tmp|xpm|ts)|spelling\.dat|Exception_to_GPL_for_Qt.txt|sqlite3.c|debian/build|ms-windows/osgeo4w|ChangeLog|src/plugins/grass/qtermwidget|src/app/gps/qwtpolar-1.0"

egrep --exclude-dir=.git --color=always "$RE" -ir .  | egrep -iv "$EX"
