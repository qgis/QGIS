# Project file for building QGIS on Windows
# This file and the QGIS source tree should reside in
# a directory named qgis_win32 in order for the qmake
# build system to work.
#
# Building for Windows requires setting the following
# environment variables:
#   GDAL -       directory containing the gdal install
#   POSTGRESQL - directory containing the postgresql install
#                (windows client only)
# qgis.pro,v 1.12 2004/07/14 18:20:11 gsherman Exp #
TEMPLATE = subdirs
SUBDIRS =  src \
           providers \
           plugins
