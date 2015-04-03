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
# A simple script to get rid of .orig and .rej files left in 
# your repository by svn. It will request confirmation before 
# deleting each file.
#
# Tim Sutton, May 2008
find . -name "*.orig" -exec rm -rf {} \;
