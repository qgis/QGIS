#!/bin/bash
###########################################################################
#    quickpackage.sh
#    ---------------------
#    Date                 : November 2010
#    Copyright            : (C) 2010 by Tim Sutton
#    Email                : tim dot linfiniti at com
###########################################################################
#                                                                         #
#   This program is free software; you can redistribute it and/or modify  #
#   it under the terms of the GNU General Public License as published by  #
#   the Free Software Foundation; either version 2 of the License, or     #
#   (at your option) any later version.                                   #
#                                                                         #
###########################################################################


# This script is just for if you want to run the nsis (under linux) part 
# of the package building process. Typically you should use 
#
# osgeo4w/creatensis.pl
#
# rather to do the complete package build process. However running this 
# script can be useful if you have manually tweaked the package contents 
# under osgeo4w/unpacked and want to create a new package based on that.
#
# Tim Sutton November 2010

makensis \
-DVERSION_NUMBER='1.7.0' \
-DVERSION_NAME='Wroclaw' \
-DSVN_REVISION='0' \
-DQGIS_BASE='QGIS' \
-DINSTALLER_NAME='QGIS-1-7-0-Setup.exe' \
-DDISPLAYED_NAME='QGIS 1.7.0' \
-DBINARY_REVISION=1 \
-DINSTALLER_TYPE=OSGeo4W \
-DPACKAGE_FOLDER=osgeo4w/unpacked \
-DSHORTNAME=qgis \
QGIS-Installer.nsi
