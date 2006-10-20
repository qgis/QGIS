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

include(../../settings.pro)
include(../ui/ui.pro)
TEMPLATE=lib
TARGET=qgis_composer
#suffix debug to target if applicable
CONFIG(debug, debug|release){
  TARGET = $$member(TARGET, 0)-debug
}
LIBS += $${GDALLIBADD}
LIBS += $${QGISCORELIBADD}
message("QGIS Core Lib: $${QGISCORELIBADD}")
message("LIBS: $${LIBS}")
DESTDIR=$${QGISLIBDIR}
#leave the next line here - it clears the Qt defines
QT =
QT += qt3support svg core gui
message("Building libs into $${DESTDIR}")

libqgis_composerHEADERS = qgscomposer.h \
		qgscomposeritem.h	\
		qgscomposerlabel.h	\
		qgscomposerpicture.h	\
		qgscomposermap.h	\
		qgscomposerscalebar.h	\
		qgscomposervectorlegend.h \
		qgscomposerview.h	\
		qgscomposition.h					

libqgis_composer_la_SOURCES =  qgscomposer.cpp \
		qgscomposeritem.cpp	       \
		qgscomposerlabel.cpp	       \
		qgscomposerpicture.cpp	       \
		qgscomposermap.cpp	       \
		qgscomposerscalebar.cpp	       \
		qgscomposervectorlegend.cpp    \
		qgscomposerview.cpp	       \
		qgscomposition.cpp					

