####################################################################
# Qmake project file for QGIS plugin
# This file is used by qmake to generate the Makefiles for building
# the QGIS copyright plugin on Windows
#
# $Id$
####################################################################
TEMPLATE = lib or app for a standalone exe
INCLUDEPATH += . \
               ..\..\qgis_win32 \
               ..\..\qgis_win32\src \
               ..\..\qgis_win32\plugins \
               include 
LIBS += ..\..\qgis_win32\src\libqgis.lib \
        libexif-9.lib
CONFIG += qt dll thread
DLLDESTDIR= ..\..\qgis_win32\win_build\lib\qgis

# Input
HEADERS += plugingui.h \
           exif2gpx.h \
           plugin.h 
INTERFACES += pluginguibase.ui
SOURCES += plugingui.cpp \
           exif2gpx.cpp \
           plugin.cpp 
