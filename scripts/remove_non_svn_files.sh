#!/bin/bash
###########################################################################
#    remove_non_svn_files.sh
#    ---------------------
#    Date                 : August 2008
#    Copyright            : (C) 2008 by Tim Sutton
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
# A simple script to get rid of files that are not 
# managed by svn. It will request confirmation before 
# deleting each file.
#

for FILE in `svn status |grep ^? | awk '{print $2}'`;do rm -i -r $FILE; done
