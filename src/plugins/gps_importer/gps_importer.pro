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
# This file builds the gps importer plugin
#

include(../../../settings.pro)
TARGET=gpsgplugin
TEMPLATE = lib
#suffix debug to target if applicable
CONFIG(debug, debug|release){
  TARGET = $$member(TARGET, 0)-debug
}

# This is a hack (thanks freddy!) because many plugins use the
# same class names and file names we need to force the compiler
# to create separate object files for them.
MYDIRNAME=gps_importer
MOC_DIR = $${OBJDIR}/moc/plugins/$${MYDIRNAME}
UI_DIR = $${OBJDIR}/ui/plugins/$${MYDIRNAME}
win32:OBJECTS_DIR = $${OBJDIR}/o/win32/plugins/$${MYDIRNAME}
INCLUDEPATH += $${OBJDIR}/ui 
INCLUDEPATH += .

LIBS += $${QGISCORELIBADD}
LIBS += $${QGISGUILIBADD}
LIBS += $${POSTGRESLIBADD}


DESTDIR=$${QGISPLUGINDIR}
QT += qt3support svg core gui xml network

message("Building libs into $${DESTDIR}")


# Input
RESOURCES += qgsgps_plugin.qrc

FORMS += qgsgpspluginguibase.ui \
              qgsgpsdevicedialogbase.ui

HEADERS +=    qgsgpsplugin.h \
              qgsbabelformat.h \
              qgsgpsdevice.h\
              qgsbabelformat.h \
              qgsgpsplugingui.h \
              qgsgpsdevicedialog.h
SOURCES +=    qgsgpsplugin.cpp \
              qgsgpsdevice.cpp \
              qgsbabelformat.cpp \
              qgsgpsplugingui.cpp \
              qgsgpsdevicedialog.cpp 

           
