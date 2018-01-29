#!/usr/bin/env bash
###########################################################################
#    chkdoclink.sh
#    ---------------------
#    Date                 : August 2017
#    Copyright            : (C) 2017 by Jorge Gustavo Rocha
#    Email                : jgr at geomaster dot pt
###########################################################################
#                                                                         #
#   This program is free software; you can redistribute it and/or modify  #
#   it under the terms of the GNU General Public License as published by  #
#   the Free Software Foundation; either version 2 of the License, or     #
#   (at your option) any later version.                                   #
#                                                                         #
###########################################################################

# This script is used to check missing links to the documentation
# This script only reports missing links
# An option argument can be used to indicate the base URL of the documentation, for example:
#
# scripts/chkdoclink.sh http://docs.qgis.org/2.18/en/docs/user_manual/
#
# When no link is indicated, the link used is:
# http://docs.qgis.org/testing/en/docs/user_manual/
#
# Nødebo, August 2017

prefix=${1:-http://docs.qgis.org/testing/en/docs/user_manual/}
find .. \( -name \*.h -o -name \*.cpp \) -exec grep -H "QgsHelp::openHelp(" \{\} \; | sed 's/:[^"]\+/\t/;s/" .\+$/"/' | sort | sed 's/^\.\.\/QGIS\///' | awk -F $'\t' '{print $1 ";" $2;}' | grep -v ";$" | sed 's/"//g' | while read line; do
    file=${line%;*}
    suffix=${line##*;}
    link=$prefix$suffix
    if ! wget --spider $link 2>/dev/null; then
        echo "Documentation missing for: $file Key: $suffix"
    fi
done

