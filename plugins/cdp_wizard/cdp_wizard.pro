####################################################################
# Qmake project file for QGIS plugin
# This file is used by qmake to generate the Makefiles for building
# the QGIS copyright plugin on Windows
#
# $Id$
####################################################################
TEMPLATE = lib or app for a standalone exe
INCLUDEPATH += . \
               $(GDAL)\include \
               ..\..\qgis_win32 \
               ..\..\qgis_win32\src \
               ..\..\qgis_win32\plugins 
LIBS += ..\..\qgis_win32\src\libqgis.lib \
        $(GDAL)\lib\gdal_i.lib
CONFIG += qt dll thread
DLLDESTDIR= ..\..\qgis_win32\win_build\lib\qgis

# Input
HEADERS += cdpwizard.h \
           cdpwizardbase.ui.h \
           climatedataprocessor.h \
           dataprocessor.h \
           filegroup.h \
           filereader.h \
           filewriter.h \
           plugin.h \
           imagewriter.h \
           meridianswitcher.h \
           testdatamaker.h
INTERFACES += cdpwizardbase.ui
SOURCES += cdpwizard.cpp \
           climatedataprocessor.cpp \
           dataprocessor.cpp \
           filegroup.cpp \
           filereader.cpp \
           filewriter.cpp \
           main.cpp \
           plugin.cpp \
           imagewriter.cpp \
           meridianswitcher.cpp \
           testdatamaker.cpp
