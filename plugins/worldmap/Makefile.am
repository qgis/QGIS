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

%.h: %.ui
	$(UIC) -o $@ $<

%.cpp: %.ui
	$(UIC) -o $@ -impl $*.h $<


plugin_LTLIBRARIES = worldmapimporterplugin.la

plugin_MOC = qgsworldmapimporter.moc.cpp		\
	     qgsworldmapimportergui.moc.cpp		\
	     qgsworldmapimporterguibase.moc.cpp

plugin_UI = qgsworldmapimporterguibase.h \
	    qgsworldmapimporterguibase.cpp 

plugin_UIC = qgsworldmapimporterguibase.ui

worldmapimporterplugin_la_SOURCES =  qgsworldmapimporter.cpp	\
			      	       qgsworldmapimporter.h		\
				       qgsworldmapimportergui.cpp	\
				       qgsworldmapimportergui.h	\
				       $(plugin_UI)			\
				       $(plugin_MOC)

BUILT_SOURCES = $(plugin_MOC) $(plugin_UI)



worldmapimporterplugin_la_LIBADD = $(QT_LDADD) ../../src/libqgis.la
worldmapimporterplugin_la_LDFLAGS = -avoid-version -module
worldmapimporterplugin_la_CXXFLAGS = $(CXXFLAGS) $(EXTRA_CXXFLAGS) $(QT_CXXFLAGS) $(DEBUG_QGIS) -I../../src/

EXTRA_DIST = $(plugin_UIC) icon.xpm

