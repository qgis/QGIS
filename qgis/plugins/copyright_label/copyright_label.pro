####################################################################
# Qmake project file for QGIS plugin
# This file is used by qmake to generate the Makefiles for building
# the QGIS copyright plugin on Windows
#
# copyright_label.pro,v 1.1 2004/06/23 04:15:54 gsherman Exp
####################################################################
TEMPLATE = lib
INCLUDEPATH += . ..\..\src \
          $(GEOS)\include
LIBS += ..\..\src\libqgis.lib \
        $(GDAL)\lib\gdal_i.lib \
        $(GEOS)\lib\geos.lib
CONFIG += qt dll thread
DLLDESTDIR= ..\..\win_build\lib\qgis

# Input
HEADERS += plugin.h \
           plugingui.h \
           pluginguibase.ui.h 
INTERFACES += pluginguibase.ui
SOURCES += plugin.cpp \
           plugingui.cpp 
