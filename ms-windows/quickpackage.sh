#!/bin/bash

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
-DVERSION_NUMBER='$major.$minor.$patch' \
-DVERSION_NAME='$release' \
-DSVN_REVISION='$revision' \
-DQGIS_BASE='Quantum GIS $release' \
-DINSTALLER_NAME='QGIS-OSGeo4W-1-6-0-Setup.exe' \
-DDISPLAYED_NAME='Quantum GIS 1.6.0' \
-DBINARY_REVISION=1 \
-DINSTALLER_TYPE=OSGeo4W \
-DPACKAGE_FOLDER=osgeo4w/unpacked \
QGIS-Installer.nsi
