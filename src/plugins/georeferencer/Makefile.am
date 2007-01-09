# Copyright (C) 2005 Gary Sherman <sherman at mrcc.com>
#  
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without 
# modifications, as long as this notice is preserved.
# 
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
#
# $Id$


if HAVE_GSL


plugindir    = ${pkglibdir}

%.moc.cpp: %.h
	$(MOC) -o $@ $<

ui_%.h: %.ui 
	$(UIC) -o $@ $<

%.qrc.cpp: %.qrc
	$(RCC) -o $@ $<

## target plug-in 
plugin_LTLIBRARIES = georefplugin.la

## qt designer user-interface files
plugin_UIC = pluginguibase.ui \
             qgspointdialogbase.ui \
             mapcoordsdialogbase.ui \
             qgsgeorefwarpoptionsdialogbase.ui

## uic created headers
plugin_UI = ui_pluginguibase.h \
            ui_qgspointdialogbase.h \
            ui_qgsgeorefwarpoptionsdialogbase.h \
            ui_mapcoordsdialogbase.h 

## MOC files
plugin_MOC = plugin.moc.cpp \
             plugingui.moc.cpp \
             qgspointdialog.moc.cpp \
             mapcoordsdialog.moc.cpp \
             qgsgeorefwarpoptionsdialog.moc.cpp


plugin_RES = georeferencer.qrc.cpp

plugin_QRC = georeferencer.qrc


georefplugin_la_SOURCES = plugin.cpp \
	qgsgeorefdatapoint.h \
	qgsgeorefdatapoint.cpp \
	plugin.h \
	plugingui.cpp \
	plugingui.h \
	qgsimagewarper.h \
	qgsimagewarper.cpp \
	qgsleastsquares.h \
	qgsleastsquares.cpp \
	qgspointdialog.h \
	qgspointdialog.cpp \
	qgsgeorefwarpoptionsdialog.h \
	qgsgeorefwarpoptionsdialog.cpp \
	mapcoordsdialog.h \
	mapcoordsdialog.cpp \
	$(plugin_UI) \
	$(plugin_RES)

nodist_georefplugin_la_SOURCES = $(plugin_MOC) $(plugin_UI)

georefplugin_la_CXXFLAGS = $(CXXFLAGS) $(EXTRA_CXXFLAGS) $(QT_CXXFLAGS) $(DEBUG_QGIS) $(GDAL_CFLAGS) $(GEOS_CFLAGS) $(GSL_CFLAGS) -I../ -I../../ui -I../../gui -I../../core/include
georefplugin_la_LDFLAGS = -avoid-version -module
georefplugin_la_LIBADD = $(QT_LDADD) $(GDAL_LDADD) $(GSL_LIBS) ../../gui/libqgis_gui.la ../../core/libqgis_core.la

BUILT_SOURCES = $(plugin_UI) $(plugin_MOC) $(plugin_RES)

CLEANFILES = $(BUILT_SOURCES)

EXTRA_DIST = $(plugin_UIC) add_point.xpm


endif
