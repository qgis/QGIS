####################################################################
# Qmake project file for QGIS data provider
# This file is used by qmake to generate the Makefile for building
# the QGIS OGR data provider on Windows
#
# ogr.pro,v 1.2 2004/08/14 01:10:09 timlinux Exp
####################################################################

################# 
# GEOS Notes    #
##########################################################################
# Geos support is currenlty provided by a custom compiled library.
# The library is compiled with vc++ and statically linked with the
# ogr provider. A dll may be supplied at a later date. GEOS source
# used in creating the library is available at geos.refractions.net.
# To compile the windows version, set the GEOS environment variable
# to point to the directory containing the include and lib subdirectories.
# The headers/lib can be downloaded from http://qgis.org/win32_geos.zip
###########################################################################

TEMPLATE = lib
INCLUDEPATH += . $(GDAL)\include \
                $(GEOS)\include \
                $(QTDIR)\include
LIBS += $(GDAL)\lib\gdal_i.lib \
                ..\..\src\libqgis.lib \
                $(GEOS)\lib\geos.lib
CONFIG += qt dll thread debug
DLLDESTDIR= ..\..\win_build\lib\qgis
#DEFINES += NOWIN32GEOS
# Input
HEADERS += qgsshapefileprovider.h
SOURCES += qgsshapefileprovider.cpp \
           ..\..\src\qgsfeature.cpp \
           ..\..\src\qgsfield.cpp \
           ..\..\src\qgsrect.cpp \
           ..\..\src\qgsfeatureattribute.cpp \
           ..\..\src\qgspoint.cpp
