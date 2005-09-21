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
#ScaleBar below to be replaced with the name of the plugin
plugin_LTLIBRARIES = libScaleBarplugin.la

%.moc.cpp: %.h
	$(MOC) -o $@ $<

if HAVE_QT3
%.uic.h: %.ui 
	$(UIC) -o $@ $<
endif

if HAVE_QT4
%.uic.h: %.ui 
	$(UIC) -o $@ $<
	../../tools/qgis-qt3to4.pl -uic $@
endif

if !NO_UIC_IMPLEMENTATIONS
%.uic.cpp: %.ui
	$(UIC) -o $@ -impl $*.uic.h $<
endif

# %.h: %.ui
# 	$(UIC) -o $@ $<
#
# %.cpp: %.ui
# 	$(UIC) -o $@ -impl $*.h $<

libScaleBarplugin_la_SOURCES =  plugin.cpp \
			      plugin.h \
			      plugingui.cpp \
			      plugingui.h \
			      $(plugin_UI)\
			      $(plugin_MOC)

plugin_MOC = plugin.moc.cpp \
	     plugingui.moc.cpp \
	     pluginguibase.moc.cpp

plugin_UI = pluginguibase.uic.h \
	    pluginguibase.uic.cpp 

plugin_UIC = pluginguibase.ui

# UI dependencies      
pluginguibase.uic.cpp: pluginguibase.uic.h pluginguibase.ui

BUILT_SOURCES = $(plugin_MOC) $(plugin_UI)


libScaleBarplugin_la_LIBADD = $(QT_LDADD) ../../src/libqgis.la
libScaleBarplugin_la_LDFLAGS = -avoid-version -module
libScaleBarplugin_la_CXXFLAGS = $(CXXFLAGS) $(EXTRA_CXXFLAGS) $(QT_CXXFLAGS) $(DEBUG_QGIS) $(GDAL_CFLAGS) $(GEOS_CFLAGS) -I../../src

CLEANFILES = $(BUILT_SOURCES)

EXTRA_DIST = $(plugin_UIC) \
	icon.xpm \
	pluginguibase.ui.h
