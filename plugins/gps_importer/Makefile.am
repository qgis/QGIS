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

PKGDATAPATH=-DPKGDATAPATH=\"$(pkgdatadir)\"

plugindir = ${pkglibdir}

plugin_LTLIBRARIES = gpsimporterplugin.la


%.moc.cpp: %.h
	$(MOC) -o $@ $<

%.h: %.ui
	$(UIC) -o $@ $<

%.cpp: %.ui
	$(UIC) -o $@ -impl $*.h $<


gpsimporterplugin_la_SOURCES = \
	qgsgpsplugin.cpp \
	qgsbabelformat.h \
	qgsbabelformat.cpp \
	qgsgpsplugingui.cpp \
	qgsgpsdevicedialog.cpp \
	$(plugin_UI)\
	$(plugin_MOC)

plugin_MOC = \
	qgsgpsplugin.moc.cpp \
	qgsgpsplugingui.moc.cpp \
	qgsgpspluginguibase.moc.cpp \
	qgsgpsdevicedialogbase.moc.cpp \
	qgsgpsdevicedialog.moc.cpp

plugin_UI = \
	qgsgpspluginguibase.h \
	qgsgpspluginguibase.cpp \
	qgsgpsdevicedialogbase.h \
	qgsgpsdevicedialogbase.cpp

plugin_UIC = \
	qgsgpspluginguibase.ui \
	qgsgpsdevicedialogbase.ui

# UI dependencies      
qgsgpspluginguibase.cpp: qgsgpspluginguibase.h qgsgpspluginguibase.ui
qgsgpsdevicedialogbase.cpp: qgsgpsdevicedialogbase.h qgsgpsdevicedialogbase.ui

BUILT_SOURCES = $(plugin_MOC) $(plugin_UI)

gpsimporterplugin_la_LIBADD = $(QT_LDADD)  $(GDAL_LDADD) ../../src/libqgis.la
gpsimporterplugin_la_CFLAGS = $(CFLAGS) $(EXTRA_CFLAGS) $(DEBUG_QGIS)
gpsimporterplugin_la_CXXFLAGS = $(CXXFLAGS) $(EXTRA_CXXFLAGS) $(QT_CXXFLAGS) $(DEBUG_QGIS) -I../../src
gpsimporterplugin_la_LDFLAGS = -avoid-version -module 


## 
## Anything that must go in the tarball gets added here
##

EXTRA_DIST = $(plugin_UIC) icon.xpm \
	qgsgpspluginguibase.h \
	qgsgpsdevicedialogbase.h \
	qgsgpsdevicedialogbase.ui.h \
	qgsgpsplugingui.h \
	qgsgpsplugin.h \
	qgsgpsdevicedialog.h
