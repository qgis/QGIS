####################################################################
# Qmake project file for QGIS data provider
# This file is used by qmake to generate the Makefile for building
# the QGIS OGR data provider on Windows
#
# ogr.pro,v 1.1 2004/07/15 01:10:40 gsherman Exp
####################################################################

TEMPLATE = lib
INCLUDEPATH += . C:\GDALBuild"\include
LIBS += C:\GDALBuild\lib\gdal_i.lib
CONFIG += qt dll thread debug
DLLDESTDIR= ..\..\win_build\lib\qgis
# Input
HEADERS += qgsshapefileprovider.h
SOURCES += qgsshapefileprovider.cpp \
           ..\..\src\qgsfeature.cpp \
           ..\..\src\qgsfield.cpp \
           ..\..\src\qgsrect.cpp \
           ..\..\src\qgsfeatureattribute.cpp \
           ..\..\src\qgspoint.cpp
