#################################################################
#
#         QMAKE Project File for Quantum GIS 
# 
#                   Tim Sutton 2006
#
# NOTE: Do not place any hard coded external paths in this file
#       all libs and includes should be specified in settings.pro
#       in the top level qgis directory.
# 
#################################################################

#
# This file builds the wfs provider
#

include(../../../settings.pro)
TARGET=wfsprovider
TEMPLATE = lib
#suffix debug to target if applicable
CONFIG(debug, debug|release){
  TARGET = $$member(TARGET, 0)-debug
}

LIBS += $${GEOSLIBADD}
LIBS += $${QGISCORELIBADD}

DESTDIR=$${QGISPROVIDERDIR}
QT += qt3support svg core gui xml network
message("Building libs into $${DESTDIR}")

CONFIG += qt dll thread debug rtti
HEADERS += qgswfsprovider.h
SOURCES += qgswfsprovider.cpp 
