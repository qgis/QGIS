####################################################################
# Qmake project file for QGIS data provider
# This file is used by qmake to generate the Makefile for building
# the QGIS OGR data provider on Windows
#
# ogr.pro,v 1.2 2004/08/14 01:10:09 timlinux Exp
####################################################################

TEMPLATE = lib
INCLUDEPATH += . %GDAL%\include
LIBS += $(GDAL)\lib\gdal_i.lib 
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
