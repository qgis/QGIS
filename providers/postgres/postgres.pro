######################################################################
# Qmake project file for building the postgres provider.
# This file is needed for building on Windows
######################################################################
# postgres.pro,v 1.2 2004/06/23 04:15:55 gsherman Exp 

TEMPLATE = lib
INCLUDEPATH += . \
                ..\..\src \
                $(POSTGRESQL)\src\interfaces\libpq \
                $(POSTGRESQL)\src\include \
                $(GEOS)\include \
                $(GDAL)\include \
                $(WINSDK)\include
LIBS += $(POSTGRESQL)\src\interfaces\libpq\Release\libpq.lib \
        ..\..\src\libqgis.lib \
        $(GEOS)\lib\geos.lib \
        $(FWTOOLS)\lib\gdal_i.lib

CONFIG += qt dll thread
DLLDESTDIR= ..\..\win_build\lib\qgis
# Input
HEADERS += qgspostgresprovider.h \
           qgspostgrescountthread.h \
           qgspostgisbox2d.h \
           qgspostgisbox3d.h \
           qgspostgresextentthread.h \
           

SOURCES += qgspostgresprovider.cpp \
           qgspostgrescountthread.cpp \
           qgspostgisbox2d.cpp \
           qgspostgisbox3d.cpp \
           qgspostgresextentthread.cpp \
           ..\..\src\qgsfeature.cpp \
           ..\..\src\qgsfield.cpp \
           ..\..\src\qgsrect.cpp \
           ..\..\src\qgsfeatureattribute.cpp \
           ..\..\src\qgspoint.cpp
