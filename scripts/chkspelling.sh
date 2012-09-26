#!/bin/bash


RE=$(echo $(cut -d: -f1 scripts/spelling.dat | sed -e 's/^/\\</; s/$/\\>|/;') | sed -e 's/| /|/g; s/|$//;')
EX="\.(svn-base|tmp|xpm|ts)|context_help|spelling\.dat|Exception_to_GPL_for_Qt.txt|sqlite3.c|debian/build|ms-windows/osgeo4w|ChangeLog|src/plugins/grass/qtermwidget|src/app/gps/qwtpolar-1.0"

egrep --exclude-dir=.git --color=always "$RE" -ir .  | egrep -iv "$EX"
egrep --exclude-dir=.git --color=always "$RE" -i resources/context_help/*-en_US
