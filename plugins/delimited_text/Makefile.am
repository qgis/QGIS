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
#DelimitedTextLayer below to be replaced with the name of the plugin
plugin_LTLIBRARIES = delimitedtextlayerplugin.la

%.moc.cpp: %.h
	$(MOC) -o $@ $<

%.h: %.ui
	$(UIC) -o $@ $<

%.cpp: %.ui
	$(UIC) -o $@ -impl $*.h $<

delimitedtextlayerplugin_la_SOURCES =  qgsdelimitedtextplugin.cpp \
			      qgsdelimitedtextplugin.h \
			      qgsdelimitedtextplugingui.cpp \
			      qgsdelimitedtextplugingui.h \
			      ../../src/qgsmaplayer.cpp\
			      ../../src/qgsvectorlayer.cpp\
						../../src/qgssymbol.cpp\
			      $(plugin_UI)\
			      $(plugin_MOC)

plugin_MOC = qgsdelimitedtextplugin.moc.cpp \
	     ../../src/qgsmaplayer.moc.cpp \
	     ../../src/qgsvectorlayer.moc.cpp \
	     qgsdelimitedtextplugingui.moc.cpp \
	     qgsdelimitedtextpluginguibase.moc.cpp

plugin_UI = qgsdelimitedtextpluginguibase.h			\
	    qgsdelimitedtextpluginguibase.cpp 

plugin_UIC = qgsdelimitedtextpluginguibase.ui


BUILT_SOURCES = $(plugin_MOC) $(plugin_UI)

AM_CXXFLAGS := $(CXXFLAGS) $(EXTRA_CXXFLAGS) $(QT_CXXFLAGS) $(DEBUG_QGIS) 

delimitedtextlayerplugin_la_LIBADD = $(QT_LDADD)
delimitedtextlayerplugin_la_LDFLAGS = -avoid-version -module

EXTRA_DIST = $(plugin_UIC) icon.xpm

