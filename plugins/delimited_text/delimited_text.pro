####################################################################
# Qmake project file for QGIS top-level plugins directory
# This file is used by qmake to generate the Makefiles for building
# QGIS plugins on Windows
#
# delimited_text.pro,v 1.1 2004/06/23 04:15:54 gsherman Exp
####################################################################
TEMPLATE = lib
INCLUDEPATH += . ..\..\src $(GDAL)\include
LIBS += $(GDAL)\lib\gdal_i.lib
DLLDESTDIR= ..\..\win_build\lib\qgis
CONFIG += qt dll thread
# Input
HEADERS += qgsdelimitedtextplugin.h \
           qgsdelimitedtextplugingui.h 
INTERFACES += qgsdelimitedtextpluginguibase.ui
SOURCES += qgsdelimitedtextplugin.cpp \
           qgsdelimitedtextplugingui.cpp 
