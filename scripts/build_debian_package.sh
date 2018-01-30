#!/bin/bash
###########################################################################
#    build_debian_package.sh
#    ---------------------
#    Date                 : July 2007
#    Copyright            : (C) 2007 by Tim Sutton
#    Email                : tim at kartoza dot com
###########################################################################
#                                                                         #
#   This program is free software; you can redistribute it and/or modify  #
#   it under the terms of the GNU General Public License as published by  #
#   the Free Software Foundation; either version 2 of the License, or     #
#   (at your option) any later version.                                   #
#                                                                         #
###########################################################################

set -x
if [ -d `pwd`/src ]
then
  #src exists so we are prolly in the right dir
  echo "good we are in  qgis checkout dir!"
else
  echo "You must run this from the top level qgis checkout dir!"
  exit 1
fi

if [ -d `pwd`/debian ]
then
  cd debian
  svn update
  cd ..
else
  svn co https://svn.qgis.org/repos/qgis/trunk/debian
fi

export DEBFULLNAME="Tim Sutton"
export DEBEMAIL=tim@linfiniti.com
#dch -v 0.9.1+svn`date +%Y%m%d`
dch -v 0.9.2rc1
fakeroot dpkg-buildpackage -kAA4D3BA997626237 
