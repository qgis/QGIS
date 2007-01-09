# Copyright (C) 2003 Gary Sherman <sherman at mrcc.com> 
#           and 2006 Tim Sutton <tim@linfiniti.com>
# 
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without 
# modifications, as long as this notice is preserved.
# 
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
#
# $Id: Makefile.am 5381 2006-04-26 15:02:28Z wonder $

#navigation below to be replaced with the name of the plugin

##
## Legacy support for pkgdatapath - you are encouraged to use
## Qt4.x resource files where possible rather!
##
if !HAVE_QTMAC
PKGDATAPATH=-DPKGDATAPATH=\"$(pkgdatadir)\"
endif

plugindir = ${pkglibdir}

#for plugin so
plugin_LTLIBRARIES = libqgis_plugin_navigation.la

## For Qt4 intermerdiary meta object compiler files
%.moc.cpp: %.h
	$(MOC) -o $@ $<

## For Qt4 User Interface Files
ui_%.h: %.ui
	$(UIC) -o $@ $<

## For Qt4 Resource Files
%.qrc.cpp: %.qrc
	$(RCC) -o $@ $<

##
##  For plugin lib
##

libqgis_plugin_navigation_la_SOURCES = navigation.cpp \
	navigationgui.cpp \
	gpspage.cpp \
	gpscore.cpp \
	positionmarker.cpp \
	routingcore.cpp \
	$(plugin_UI)\
	$(plugin_MOC)\
	$(plugin_RES)

	     
plugin_MOC = navigation.moc.cpp \
    navigationgui.moc.cpp \
    gpscore.moc.cpp \
    routingcore.moc.cpp

plugin_UI = ui_navigationgui.h \
            ui_gpspage.h

plugin_RES = navigation.qrc.cpp


BUILT_SOURCES = $(plugin_MOC) $(plugin_UI) $(plugin_RES)


libqgis_plugin_navigation_la_LIBADD = $(QT_LDADD) \
			    $(GDAL_LDADD) \
			    $(GPSD_LDADD) \
			    ../../core/libqgis_core.la \
			    ../../gui/libqgis_gui.la
libqgis_plugin_navigation_la_CFLAGS = $(CFLAGS) $(EXTRA_CFLAGS) $(DEBUG_QGIS)
libqgis_plugin_navigation_la_CXXFLAGS = $(CXXFLAGS) \
			      $(EXTRA_CXXFLAGS) \
			      $(QT_CXXFLAGS) \
			      $(DEBUG_QGIS) \
			      $(GDAL_CFLAGS) \
			      $(GEOS_CFLAGS) \
			      -I../../core/include \
			      -I../../ui \
			      -I../../gui

libqgis_plugin_navigation_la_LDFLAGS = -avoid-version -module
