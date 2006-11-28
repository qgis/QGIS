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
# This file builds the wfs plugin
#

include(../../../settings.pro)
TARGET=grassplugin
TEMPLATE = lib
#suffix debug to target if applicable
CONFIG(debug, debug|release){
  TARGET = $$member(TARGET, 0)-debug
}

LIBS += $${QGISCORELIBADD}
LIBS += $${QGISGUILIBADD}
LIBS += $${QGISPROJECTIONSELECTORLIBADD}
LIBS += $${GRASSLIBADD}
LIBS += $${PROJLIBADD}
LIBS += $${GDALLIBADD}
LIBS += $${QGISGRASSPROVIDERLIBADD}
DESTDIR=$${QGISPLUGINDIR}
QT += qt3support svg core gui xml network
message("Building libs into $${DESTDIR}")

HEADERS += qgsgrassattributes.h		\
           qgsgrassbrowser.h		\
           qgsgrassedit.h		\
	   qgsgrassedittools.h		\
           qgsgrassmapcalc.h		\
           qgsgrassmodel.h		\
           qgsgrassmodule.h		\
           qgsgrassnewmapset.h		\
           qgsgrassplugin.h		\
	   qgsgrassregion.h		\
	   qgsgrassselect.h		\
	   qgsgrassshell.h		\
	   qgsgrasstools.h		\
           qgsgrassutils.h

FORMS +=   qgsgrassattributesbase.ui	\
	   qgsgrasseditbase.ui		\
	   qgsgrassmapcalcbase.ui	\
	   qgsgrassmodulebase.ui	\
	   qgsgrassnewmapsetbase.ui	\
	   qgsgrassregionbase.ui	\
	   qgsgrassselectbase.ui	\
	   qgsgrassshellbase.ui

SOURCES += qgsgrassattributes.cpp	\
	   qgsgrassbrowser.cpp		\
	   qgsgrassedit.cpp		\
	   qgsgrassedittools.cpp	\
	   qgsgrassmapcalc.cpp		\
	   qgsgrassmodel.cpp		\
	   qgsgrassmodule.cpp		\
	   qgsgrassnewmapset.cpp	\
	   qgsgrassplugin.cpp		\
	   qgsgrassregion.cpp		\
	   qgsgrassselect.cpp		\
	   qgsgrassshell.cpp		\
	   qgsgrasstools.cpp		\
	   qgsgrassutils.cpp
