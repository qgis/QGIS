#!/bin/bash
###########################################################################
#    make-tarball.sh
#    ---------------------
#    Date                 : October 2011
#    Copyright            : (C) 2011 by Tim Sutton
#    Email                : tim at kartoza dot com
###########################################################################
#                                                                         #
#   This program is free software; you can redistribute it and/or modify  #
#   it under the terms of the GNU General Public License as published by  #
#   the Free Software Foundation; either version 2 of the License, or     #
#   (at your option) any later version.                                   #
#                                                                         #
###########################################################################


# A simple script to create a release tarball
#
#  Tim Sutton, October 2011

TAG_OR_BRANCH="master"
VERSION="1.8.0-dev"

git archive --format=tar --prefix=qgis-${VERSION}/ ${TAG_OR_BRANCH=} | \
   bzip2 > /tmp/qgis-${VERSION}.tar.bz2
md5sum /tmp/qgis-${VERSION}.tar.bz2 > \
    /tmp/qgis-${VERSION}.tar.bz2.md5
