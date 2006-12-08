######################################################################
# Qmake project file for building the postgres provider.
# This file is needed for building on Windows
######################################################################
# wms.pro,v 1.0 2005/03/15 00:00:00 morb_au Exp 
include(../../../settings.pro)
TARGET=wmsprovider
TEMPLATE = lib
#suffix debug to target if applicable
CONFIG(debug, debug|release){
  TARGET = $$member(TARGET, 0)-debug
}
LIBS += $${QGISCORELIBADD}
#LIBS += $${QGISGUILIBADD}
DESTDIR=$${QGISPROVIDERDIR}
QT += qt3support svg core gui xml network
message("Building libs into $${DESTDIR}")


HEADERS += qgswmsprovider.h
SOURCES += qgswmsprovider.cpp
