####################################################################
# Qmake project file for QGIS plugin
# This file is used by qmake to generate the Makefiles for building
# the QGIS copyright plugin on Windows
#
# $Id$
####################################################################

#
# Note this plugin should not nwd to link against gdal - but wont build without it :-(
#      although even when it does build it wont load
#

TEMPLATE = lib or app for a standalone exe
INCLUDEPATH += . %GDAL%\include ..\..\qgis_win32 ..\..\qgis_win32\src ..\..\qgis_win32\plugins 
LIBS += ..\..\qgis_win32\src\libqgis.lib $(GDAL)\lib\gdal_i.lib 
CONFIG += qt dll thread
DLLDESTDIR= ..\..\qgis_win32\win_build\lib\qgis

# Input
HEADERS += plugin.h \
           pluginguibase.ui.h \
           plugingui.h \
           httpdaemon.h \
           plugin.h 
INTERFACES += pluginguibase.ui
SOURCES += plugin.cpp \
           plugingui.cpp \
           httpdaemon.cpp 
