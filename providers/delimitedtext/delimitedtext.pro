####################################################################
# Qmake project file for QGIS data provider
# This file is used by qmake to generate the Makefile for building
# the QGIS delimited text data provider on Windows
#
# delimitedtext.pro,v 1.1 2004/07/15 01:10:40 gsherman Exp
####################################################################

TEMPLATE = lib
INCLUDEPATH += .
LIBS += 
CONFIG += qt dll thread
DLLDESTDIR= ..\..\win_build\lib\qgis

# Input
HEADERS += qgsdelimitedtextprovider.h
SOURCES += qgsdelimitedtextprovider.cpp \
           ..\..\src\qgsfeature.cpp \
           ..\..\src\qgsfield.cpp \
           ..\..\src\qgsrect.cpp \
           ..\..\src\qgsfeatureattribute.cpp \
           ..\..\src\qgspoint.cpp
