#!/bin/bash
###########################################################################
#    astyle-rollback.sh
#    ---------------------
#    Date                 : August 2008
#    Copyright            : (C) 2008 by Juergen E. Fischer
#    Email                : jef at norbit dot de
###########################################################################
#                                                                         #
#   This program is free software; you can redistribute it and/or modify  #
#   it under the terms of the GNU General Public License as published by  #
#   the Free Software Foundation; either version 2 of the License, or     #
#   (at your option) any later version.                                   #
#                                                                         #
###########################################################################


set -e

find . \( -name "*.astyle" -o -name "*.iostream" \) -exec rm {} \;
svn revert -R .
svn update
patch -p0 --dry-run <qgslogger-before.diff
patch -p0 <qgslogger-before.diff
mv qgslogger-before.diff qgslogger-before.diff.orig
svn diff >qgslogger-before.diff
diff -u qgslogger-before.diff.orig qgslogger-before.diff
