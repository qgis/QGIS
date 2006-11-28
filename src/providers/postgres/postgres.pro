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
TARGET=postgresprovider
TEMPLATE = lib
#suffix debug to target if applicable
CONFIG(debug, debug|release){
  TARGET = $$member(TARGET, 0)-debug
}
LIBS += $${GDALLIBADD}
LIBS += $${GEOSLIBADD}
LIBS += $${POSTGRESLIBADD}
LIBS += $${QGISCORELIBADD}
LIBS += $${QGISGUILIBADD}
LIBS += $${QGISPROJECTIONSELECTORLIBADD}
DESTDIR=$${QGISPROVIDERDIR}
QT += qt3support svg core gui xml network
message("Building libs into $${DESTDIR}")

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
