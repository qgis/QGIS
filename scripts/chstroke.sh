#!/bin/bash
###########################################################################
#    chstroke.sh
#    ---------------------
#    Date                 : November 2009
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


# Globaly change given value of stroke-width parameter in SVG files
# to a new value

# Parameters: old_width new_width

for F in `ls *.svg`
do
    cp $F svg.tmp
    cat svg.tmp | sed "s/stroke-width:$1/stroke-width:$2/" > $F
done

rm svg.tmp
