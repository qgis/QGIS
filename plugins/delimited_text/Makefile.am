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


# DelimitedTextLayer below to be replaced with the name of the plugin
plugin_LTLIBRARIES = delimitedtextplugin.la

plugin_MOC = qgsdelimitedtextplugin.moc.cpp		\
	     qgsdelimitedtextplugingui.moc.cpp		\
	     qgsdelimitedtextpluginguibase.moc.cpp

plugin_UI = qgsdelimitedtextpluginguibase.uic.h \
	    qgsdelimitedtextpluginguibase.uic.cpp 

plugin_UIC = qgsdelimitedtextpluginguibase.ui

delimitedtextplugin_la_SOURCES =  qgsdelimitedtextplugin.cpp	\
			      	       qgsdelimitedtextplugin.h		\
				       qgsdelimitedtextplugingui.cpp	\
				       qgsdelimitedtextplugingui.h	\
				       $(plugin_UI)			\
				       $(plugin_MOC)

BUILT_SOURCES = $(plugin_MOC) $(plugin_UI)



delimitedtextplugin_la_LIBADD = $(QT_LDADD) $(GDAL_LDADD) ../../src/libqgis.la
delimitedtextplugin_la_LDFLAGS = -avoid-version -module
delimitedtextplugin_la_CXXFLAGS = $(CXXFLAGS) $(EXTRA_CXXFLAGS) $(QT_CXXFLAGS) $(DEBUG_QGIS) $(GDAL_CFLAGS) $(GEOS_CFLAGS) -I../../src/

EXTRA_DIST = $(plugin_UIC) icon.xpm

