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

include(../../../settings.pro)
TEMPLATE=lib
TARGET=qgis_projectionselector
#suffix debug to target if applicable
CONFIG(debug, debug|release){
  TARGET = $$member(TARGET, 0)-debug
}
LIBS += $${GDALLIBADD}
LIBS += $${SQLITELIBADD}
LIBS += $${PROJLIBADD}
LIBS += $${QGISCORELIBADD}
message("QGIS Core Lib: $${QGISCORELIBADD}")
message("LIBS: $${LIBS}")
DESTDIR=$${QGISLIBDIR}
QT += core gui qt3support
QT = $$unique(QT)

FORMS += qgsprojectionselectorbase.ui
HEADERS += qgsprojectionselector.h 
SOURCES += qgsprojectionselector.cpp
