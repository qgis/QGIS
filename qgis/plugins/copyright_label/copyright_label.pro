####################################################################
# Qmake project file for QGIS plugin
# This file is used by qmake to generate the Makefiles for building
# the QGIS copyright plugin on Windows
#
# copyright_label.pro,v 1.1 2004/06/23 04:15:54 gsherman Exp
####################################################################
TEMPLATE = lib
INCLUDEPATH += . ..\..\src \
          $(GEOS)\include \
          $(WINSDK)\Include \
          $(FWTOOLS)\include
LIBS += ..\..\src\libqgis.lib \
        $(FWTOOLS)\lib\gdal_i.lib \
        $(FWTOOLS)\lib\proj_i.lib \
        $(GEOS)\lib\geos.lib \
        $(SQLITE3)\sqlite3.lib \
        $(POSTGRESQL)\src\interfaces\libpq\Release\libpq.lib \
        ..\..\widgets\projectionselector\projectionselector.lib \
        ..\..\src\libqgis.lib 

DLLDESTDIR= ..\..\win_build\lib\qgis
CONFIG += qt dll thread

# Input
HEADERS += plugin.h \
           plugingui.h \
           pluginguibase.ui.h 
INTERFACES += pluginguibase.ui
SOURCES += plugin.cpp \
           plugingui.cpp

