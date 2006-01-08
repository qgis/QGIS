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

if HAVE_EXPAT

plugindir = ${pkglibdir}

plugin_LTLIBRARIES = gpsimporterplugin.la


%.moc.cpp: %.h
	$(MOC) -o $@ $<

ui_%.h: %.ui 
	$(UIC) -o $@ $<

%.qrc.cpp: %.qrc
	$(RCC) -o $@ $<

gpsimporterplugin_la_SOURCES = \
	qgsgpsplugin.cpp \
	qgsbabelformat.h \
	qgsbabelformat.cpp \
	qgsgpsdevice.h \
	qgsgpsdevice.cpp \
	qgsgpsplugingui.cpp \
	qgsgpsdevicedialog.cpp \
	$(plugin_UI)\
	$(plugin_MOC) \
	$(plugin_QRC) \
	$(plugin_RES)

plugin_MOC = \
	qgsgpsplugin.moc.cpp \
	qgsbabelformat.moc.cpp \
	qgsgpsdevice.moc.cpp \
	qgsgpsplugingui.moc.cpp \
	qgsgpsdevicedialog.moc.cpp

plugin_UI = \
	ui_qgsgpspluginguibase.h \
	ui_qgsgpsdevicedialogbase.h 

plugin_UIC = \
	qgsgpspluginguibase.ui \
	qgsgpsdevicedialogbase.ui

plugin_RES = qgsgps_plugin.qrc.cpp

plugin_QRC = qgsgps_plugin.qrc

BUILT_SOURCES = $(plugin_MOC) $(plugin_UI)

gpsimporterplugin_la_LIBADD = $(QT_LDADD)  $(GDAL_LDADD)  ../../src/libqgis_gui.la ../../src/core/libqgis_core.la
gpsimporterplugin_la_CFLAGS = $(CFLAGS) $(EXTRA_CFLAGS) $(DEBUG_QGIS)
gpsimporterplugin_la_CXXFLAGS = $(CXXFLAGS) $(EXTRA_CXXFLAGS) $(QT_CXXFLAGS) $(DEBUG_QGIS) $(GDAL_CFLAGS) $(GEOS_CFLAGS) -I../../src/core -I../../src/ui -I../../src
gpsimporterplugin_la_LDFLAGS = -avoid-version -module 

CLEANFILES = $(BUILT_SOURCES)

## 
## Anything that must go in the tarball gets added here
##

EXTRA_DIST = $(plugin_UIC) icon.xpm \
	ui_qgsgpspluginguibase.h \
	ui_qgsgpsdevicedialogbase.h \
	ui_qgsgpsplugingui.h \
	qgsgpsplugin.h \
	qgsgpsdevicedialog.h




endif
