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


%.moc.cpp: %.h
	$(MOC) -o $@ $<

ui_%.h: %.ui 
	$(UIC) -o $@ $<

%.qrc.cpp: %.qrc
	$(RCC) -o $@ $<

# DelimitedTextLayer below to be replaced with the name of the plugin
plugin_LTLIBRARIES = delimitedtextplugin.la

plugin_MOC = qgsdelimitedtextplugin.moc.cpp		\
	     qgsdelimitedtextplugingui.moc.cpp		

plugin_UI = ui_qgsdelimitedtextpluginguibase.h 

plugin_UIC = qgsdelimitedtextpluginguibase.ui

plugin_QRC = delimited_text.qrc
 
plugin_RES = delimited_text.qrc.cpp

delimitedtextplugin_la_SOURCES =  qgsdelimitedtextplugin.cpp	\
			      	       qgsdelimitedtextplugin.h		\
				       qgsdelimitedtextplugingui.cpp	\
				       qgsdelimitedtextplugingui.h	\
				       $(plugin_UI)			\
				       $(plugin_MOC) \
							 $(plugin_QRC) \
							 $(plugin_RES)


BUILT_SOURCES = $(plugin_MOC) $(plugin_UI) $(plugin_RES)



delimitedtextplugin_la_LIBADD = $(QT_LDADD) $(GDAL_LDADD) ../../src/libqgis.la
delimitedtextplugin_la_LDFLAGS = -avoid-version -module
delimitedtextplugin_la_CXXFLAGS = $(CXXFLAGS) $(EXTRA_CXXFLAGS) $(QT_CXXFLAGS) $(DEBUG_QGIS) $(GDAL_CFLAGS) $(GEOS_CFLAGS) -I../../src/

CLEANFILES = $(BUILT_SOURCES)

EXTRA_DIST = $(plugin_UIC) icon.xpm

