######################################################################
# Qmake project file for building the postgres provider.
# This file is needed for building on Windows
######################################################################
# wms.pro,v 1.0 2005/03/15 00:00:00 morb_au Exp 

TEMPLATE = lib
INCLUDEPATH += . \
                ..\..\src \
LIBS += ..\..\src\libqgis.lib \
        
CONFIG += qt dll thread
DLLDESTDIR= ..\..\win_build\lib\qgis
# Input
HEADERS += qgshttptransaction.h \
	qgswmsprovider.h
SOURCES += qgshttptransaction.cpp \
	qgswmsprovider.cpp