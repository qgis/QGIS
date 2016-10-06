#!/bin/bash
###########################################################################
#    remove_git_confict_files.sh
#    ---------------------
#    Date                 : April 2012
#    Copyright            : (C) 2012 by Tim Sutton
#    Email                : tim at kartoza dot com
###########################################################################
#                                                                         #
#   This program is free software; you can redistribute it and/or modify  #
#   it under the terms of the GNU General Public License as published by  #
#   the Free Software Foundation; either version 2 of the License, or     #
#   (at your option) any later version.                                   #
#                                                                         #
###########################################################################

#
# A simple script to get rid of QGIS related temporary files left in 
# your QGIS source folder by git

# Tim Sutton, May 2008
find . \
  \( \
       -name "*.orig" \
    -o -name "*.prepare" \
    -o -name "*.sortinc" \
    -o -name "*.nocopyright" \
    -o -name "astyle*.diff" \
    -o -name "sha-*.diff" \
    -o -name "*.astyle" \
    -o -name "sha*.diff" \
    -o -name "*.bom" \
    -o -name "*.bak" \
    -o -name "*.rej" \
    -o -name "*.orig" \
    -o -name "*.new" \
    -o -name "*~" \
  \) \
  -print \
  -delete
