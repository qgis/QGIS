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
plugin_LTLIBRARIES = libopenmodellerplugin.la

%.moc.cpp: %.h
	$(MOC) -o $@ $<

%.h: %.ui
	$(UIC) -o $@ $<

%.cpp: %.ui
	$(UIC) -o $@ -impl $*.h $<

libopenmodellerplugin_la_SOURCES =  plugin.cpp \
				        plugin.h \
				        openmodellergui.cpp \
				        openmodellergui.h \
				        ../../src/qgsmaplayer.cpp\
				        ../../src/qgsrasterlayer.cpp\
				        $(openmodeller_UI)\
				        $(openmodeller_MOC)

openmodeller_MOC = plugin.moc.cpp \
		../../src/qgsmaplayer.moc.cpp \
		../../src/qgsrasterlayer.moc.cpp \
		openmodellergui.moc.cpp \
		openmodellerguibase.moc.cpp

openmodeller_UI = openmodellerguibase.h \
		  openmodellerguibase.cpp 

openmodeller_UIC = openmodellerguibase.ui

# UI dependencies			
openmodellerguibase.cpp: openmodellerguibase.h openmodellerguibase.ui

BUILT_SOURCES = $(openmodeller_MOC) $(openmodeller_UI)

AM_CXXFLAGS := $(CXXFLAGS) $(EXTRA_CXXFLAGS) $(QT_CXXFLAGS) $(DEBUG_QGIS) -I../../src

#-lom is libopenmodeller which should be in to /usr/lib/libom.so.0.0
libopenmodellerplugin_la_LIBADD = $(QT_LDADD) -L/usr/lib/ -lom -lm
libopenmodellerplugin_la_LDFLAGS = -avoid-version -module

EXTRA_DIST = $(openmodeller_UIC) icon_om.xpm

