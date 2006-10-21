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
# This file builds the gui library - the app is built in a separate pro file
#

include(../../../settings.pro)
TARGET=ogrprovider
TEMPLATE = lib
#suffix debug to target if applicable
CONFIG(debug, debug|release){
  TARGET = $$member(TARGET, 0)-debug
}
LIBS += $${GDALLIBADD}
LIBS += $${GEOSLIBADD}
#LIBS += $${PROJLIBADD}
LIBS += $${QGISCORELIBADD}
LIBS += $${QGISGUILIBADD}
LIBS += $${QGISPROJECTIONSELECTORLIBADD}
DESTDIR=$${QGISPROVIDERDIR}
QT += qt3support svg core gui xml network
message("Building libs into $${DESTDIR}")

CONFIG += qt dll thread debug rtti
HEADERS += qgsogrprovider.h
SOURCES += qgsogrprovider.cpp 
