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
#CommunityRegPlugin below to be replaced with the name of the plugin
plugin_LTLIBRARIES = communityregplugin.la

%.moc.cpp: %.h
	$(MOC) -o $@ $<

%.h: %.ui
	$(UIC) -o $@ $<

%.cpp: %.ui
	$(UIC) -o $@ -impl $*.h $<

communityregplugin_la_SOURCES =  plugin.cpp \
			      plugin.h \
			      plugingui.cpp \
			      plugingui.h \
			      qgslocationcapturewidget.cpp \
			      qgslocationcapturewidget.h \
			      $(plugin_UI)\
			      $(plugin_MOC)

plugin_MOC = plugin.moc.cpp \
	     plugingui.moc.cpp \
	     pluginguibase.moc.cpp \
	     qgslocationcapturewidget.moc.cpp \
	     qgslocationcapturewidgetbase.moc.cpp

plugin_UI = pluginguibase.h \
	    pluginguibase.cpp \
	    qgslocationcapturewidgetbase.h \
	    qgslocationcapturewidgetbase.cpp

plugin_UIC = pluginguibase.ui \
	     qgslocationcapturewidgetbase.ui


# UI dependencies      
pluginguibase.cpp: pluginguibase.h \
	           pluginguibase.ui\
		   qgslocationcapturewidgetbase.h \
		   qgslocationcapturewidgetbase.ui 

BUILT_SOURCES = $(plugin_MOC) $(plugin_UI)


communityregplugin_la_LIBADD = $(QT_LDADD) $(GDAL_LDADD) ../../src/libqgis.la
communityregplugin_la_LDFLAGS = -avoid-version -module
communityregplugin_la_CXXFLAGS = $(CXXFLAGS) $(EXTRA_CXXFLAGS) $(QT_CXXFLAGS) $(DEBUG_QGIS) -I../../src

EXTRA_DIST = $(plugin_UIC) icon.xpm

