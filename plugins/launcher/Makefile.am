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

# Ganked from qgis/plugins/plugin_template/Makefile.am

libdir    = ${prefix}/lib/qgis
pkglibdir = ${libdir}

%.moc.cpp: %.h
	$(MOC) -o $@ $<

%.moc.uic.cpp: %.uic.h
	$(MOC) -o $@ $<

%.uic.h: %.ui 
	$(UIC) -o $@ $<

%.uic.cpp: %.ui
	$(UIC) -o $@ -impl $*.uic.h $<

## target plug-in 
lib_LTLIBRARIES = libQgsLauncherPlugin.la

## qt designer user-interface files
plugin_UI = qgslauncherpluginguibase.ui

## uic created headers
plugin_UIHEADERS = $(plugin_UI:.ui=.uic.h)

## uic created source files
plugin_UISOURCES = $(plugin_UI:.ui=.uic.cpp)

## uic created MOC files
plugin_UIMOC = $(plugin_UI:.ui=.moc.uic.cpp)

## MOC files
plugin_MOC = qgslauncherplugin.moc.cpp qgslauncherplugingui.moc.cpp


libQgsLauncherPlugin_la_SOURCES = qgslauncherplugin.cpp \
	qgslauncherplugin.h \
	qgslauncherplugingui.cpp \
	qgslauncherplugingui.h \
	$(plugin_UISOURCES) \
	$(plugin_UIHEADERS) \
	$(plugin_UIMOC)

nodist_libQgsLauncherPlugin_la_SOURCES =	$(plugin_MOC)

libQgsLauncherPlugin_la_CXXFLAGS := $(CXXFLAGS) $(EXTRA_CXXFLAGS) $(QT_CXXFLAGS) $(GDAL_CFLAGS) $(QGIS_CXXFLAGS)
libQgsLauncherPlugin_la_LDFLAGS = -avoid-version -module $(QGIS_LDADD) $(QT_LDADD)  $(GDAL_LDADD) -lqgis

BUILT_SOURCES = $(plugin_UIHEADERS) $(plugin_UISOURCES) $(plugin_UIMOC) $(plugin_MOC)
CLEANFILES = $(BUILT_SOURCES)

EXTRA_DIST = $(plugin_UI) icon_wmi.xpm
