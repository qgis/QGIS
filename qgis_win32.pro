####################################################################
# Qmake project file for QGIS top-level source directory
# This file is used by qmake to generate the Makefiles for building
# QGIS on Windows
# When building on Windows, the QGIS source should reside in a 
# directory named qgis_win32
#
# qgis_win32.pro,v 1.2 2004/06/23 04:15:54 gsherman Exp
####################################################################

TEMPLATE = subdirs
SUBDIRS =  widgets \
           src \
           providers \
           plugins
