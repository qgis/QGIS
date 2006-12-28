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
# This file builds the georeferencer plugin
#

include(../../../settings.pro)
TARGET=georeferencerplugin
TEMPLATE = lib
#suffix debug to target if applicable
CONFIG(debug, debug|release){
  TARGET = $$member(TARGET, 0)-debug
}

# This is a hack (thanks freddy!) because many plugins use the
# same class names and file names we need to force the compiler
# to create separate object files for them.
MYDIRNAME=georeferencer
MOC_DIR = $${OBJDIR}/moc/plugins/$${MYDIRNAME}
UI_DIR = $${OBJDIR}/ui/plugins/$${MYDIRNAME}
win32:OBJECTS_DIR = $${OBJDIR}/o/win32/plugins/$${MYDIRNAME}
INCLUDEPATH += . 
INCLUDEPATH += $${OBJDIR}/ui 
#INCLUDEPATH += $${GEOSINCLUDE}

LIBS += $${QGISCORELIBADD}
LIBS += $${QGISGUILIBADD}
LIBS += $${GSLLIBADD}
LIBS += $${GDALLIBADD}


DESTDIR=$${QGISPLUGINDIR}
QT += qt3support svg core gui xml network

message("Building libs into $${DESTDIR}")

RESOURCES += georeferencer.qrc

FORMS += mapcoordsdialogbase.ui \
         pluginguibase.ui \
         qgsgeorefwarpoptionsdialogbase.ui \
         qgspointdialogbase.ui


HEADERS += mapcoordsdialog.h \
         plugin.h \
         plugingui.h \
         qgsgeorefdatapoint.h \
         qgsgeorefwarpoptionsdialog.h \
         qgsimagewarper.h \
         qgsleastsquares.h \
         qgspointdialog.h

SOURCES += mapcoordsdialog.cpp \
         plugin.cpp \
         plugingui.cpp \
         qgsgeorefdatapoint.cpp \
         qgsgeorefwarpoptionsdialog.cpp \
         qgsimagewarper.cpp \
         qgsleastsquares.cpp \
         qgspointdialog.cpp
