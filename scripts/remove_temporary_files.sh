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
# your QGIS source folder by git and the prepare-commit script
# if you don't want to use "prepare-commit.sh -c"

# Tim Sutton, May 2008
find . -name "*.orig" -delete
find . -name "*.prepare" -delete
find . -name "astyle*.diff" -delete
find . -name "*.astyle" -delete
find . -name "sha*.diff" -delete


