####################################################################
# Qmake project file for grid_maker plugin
# This file is used by qmake to generate the Makefiles for building
# the QGIS plugin on Windows
#
# grid_maker.pro,v 1.1 2004/06/23 04:15:54 gsherman Exp
####################################################################
TEMPLATE = lib
INCLUDEPATH += . ..\..\src \
              $(WINSDK)\Include \
              $(FWTOOLS)\include
LIBS += $(FWTOOLS)\lib\gdal_i.lib \
        $(FWTOOLS)\lib\proj_i.lib \
        ..\..\src\libqgis.lib \
        $(POSTGRESQL)\src\interfaces\libpq\Release\libpq.lib \
        ..\..\widgets\projectionselector\projectionselector.lib \
        $(SQLITE3)\sqlite3.lib \
        $(GEOS)\lib\geos.lib 
CONFIG += qt dll thread
DLLDESTDIR= ..\..\win_build\lib\qgis

# Input
HEADERS += graticulecreator.h \
           plugin.h \
           plugingui.h \
           pluginguibase.ui.h \
           shapefile.h \
           utils.h 
INTERFACES += pluginguibase.ui
SOURCES += dbfopen.c \
           graticulecreator.cpp \
           plugin.cpp \
           plugingui.cpp \
           shpopen.c \
           utils.c 
