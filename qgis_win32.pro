####################################################################
# Qmake project file for QGIS top-level source directory
# This file is used by qmake to generate the Makefiles for building
# QGIS on Windows
# When building on Windows, the QGIS source should reside in a 
# directory named qgis_win32
#
# $Id
#

TEMPLATE = subdirs
SUBDIRS =  src \
           providers \
           plugins
