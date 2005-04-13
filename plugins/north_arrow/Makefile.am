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

plugindir = ${pkglibdir}
if !HAVE_QTMAC
PKGDATAPATH=-DPKGDATAPATH=\"$(pkgdatadir)\"
endif
#NorthArrow below to be replaced with the name of the plugin
plugin_LTLIBRARIES = northarrowplugin.la

%.moc.cpp: %.h
	$(MOC) -o $@ $<

%.h: %.ui
	$(UIC) -o $@ $<

%.cpp: %.ui
	$(UIC) -o $@ -impl $*.h $<

northarrowplugin_la_SOURCES =  plugin.cpp \
			      plugin.h \
			      plugingui.cpp \
			      plugingui.h \
			      $(plugin_UI)\
			      $(plugin_MOC)

plugin_MOC = plugin.moc.cpp \
	     plugingui.moc.cpp \
	     pluginguibase.moc.cpp

plugin_UI = pluginguibase.h \
	    pluginguibase.cpp 

plugin_UIC = pluginguibase.ui

# UI dependencies      
pluginguibase.cpp: pluginguibase.h pluginguibase.ui

BUILT_SOURCES = $(plugin_MOC) $(plugin_UI)


northarrowplugin_la_LIBADD = $(QT_LDADD) $(GDAL_LDADD) ../../src/libqgis.la
northarrowplugin_la_LDFLAGS = -avoid-version -module
northarrowplugin_la_CXXFLAGS = $(CXXFLAGS) $(EXTRA_CXXFLAGS) $(QT_CXXFLAGS) $(DEBUG_QGIS) $(PKGDATAPATH) -I../../src

EXTRA_DIST = $(plugin_UIC) icon.xpm \
        pluginguibase.ui.h

