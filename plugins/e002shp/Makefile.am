# Copyright (C) 2003 Gary Sherman <sherman at mrcc.com>
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


plugindir    = ${prefix}/lib/qgis
pkgplguindir = ${plugindir}

%.moc.cpp: %.h
	$(MOC) -o $@ $<

%.moc.uic.cpp: %.uic.h
	$(MOC) -o $@ $<

%.uic.h: %.ui 
	$(UIC) -o $@ $<

%.uic.cpp: %.ui
	$(UIC) -o $@ -impl $*.uic.h $<

## target plug-in 
plugin_LTLIBRARIES = e002shp.la

## qt designer user-interface files
plugin_UI = pluginguibase.ui

## uic created headers
plugin_UIHEADERS = $(plugin_UI:.ui=.uic.h)

## uic created source files
plugin_UISOURCES = $(plugin_UI:.ui=.uic.cpp)

## uic created MOC files
plugin_UIMOC = $(plugin_UI:.ui=.moc.uic.cpp)

## MOC files
plugin_MOC = plugin.moc.cpp plugingui.moc.cpp


e002shp_la_SOURCES = plugin.cpp \
	plugin.h \
	plugingui.cpp \
	plugingui.h \
	$(plugin_UISOURCES) \
	$(plugin_UIHEADERS) \
	$(plugin_UIMOC)
	
nodist_e002shp_la_SOURCES =	$(plugin_MOC)

e002shp_la_CXXFLAGS = $(CXXFLAGS) $(EXTRA_CXXFLAGS) $(QT_CXXFLAGS) $(QGIS_CXXFLAGS) $(GDAL_CFLAGS)
e002shp_la_LDFLAGS = -avoid-version -module $(QGIS_LDADD) $(QT_LDADD) $(GDAL_LDADD)

BUILT_SOURCES = $(plugin_UIHEADERS) $(plugin_UISOURCES) $(plugin_UIMOC) $(plugin_MOC)
CLEANFILES = $(BUILT_SOURCES)

EXTRA_DIST = $(plugin_UI) icon.xpm





