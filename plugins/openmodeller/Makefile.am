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

# Ganked from qgis/plugins/plugin_template/Makefile.am

libdir    = ${prefix}/lib/qgis
pkglibdir = ${libdir}

%.moc.cpp: %.h
	$(MOC) -o $@ $<

%.moc.uic.cpp: %.uic.h
	$(MOC) -o $@ $<

%.uic.h: %.ui 
	$(UIC) -o $@ $<

%.uic.cpp: %.ui
	$(UIC) -o $@ -impl $*.uic.h $<

## target plug-in 
lib_LTLIBRARIES = libopenmodellerplugin.la
## target binary file
bin_PROGRAMS = omgui

## qt designer user-interface files
plugin_UI = openmodellerguibase.ui

## uic created headers
plugin_UIHEADERS = $(plugin_UI:.ui=.uic.h)

## uic created source files
plugin_UISOURCES = $(plugin_UI:.ui=.uic.cpp)

## uic created MOC files
plugin_UIMOC = $(plugin_UI:.ui=.moc.uic.cpp)

## MOC files
plugin_MOC = plugin.moc.cpp openmodellergui.moc.cpp


libopenmodellerplugin_la_SOURCES = plugin.cpp \
	plugin.h \
	openmodellergui.cpp \
	openmodellergui.h \
	request_file.cpp \
	occurrences_file.cpp \
	imagewriter.cpp \
	$(plugin_UISOURCES) \
	$(plugin_UIHEADERS) \
	$(plugin_UIMOC)
	
nodist_libopenmodellerplugin_la_SOURCES =	$(plugin_MOC)

libopenmodellerplugin_la_CXXFLAGS := $(CXXFLAGS) $(EXTRA_CXXFLAGS) $(QT_CXXFLAGS) $(QGIS_CXXFLAGS) $(GDAL_CFLAGS) $(OM_CXXFLAGS) -DQGISDATAPATH=\"$(QGISDATAPATH)\"
libopenmodellerplugin_la_LDFLAGS = -avoid-version -module $(QGIS_LDADD) $(QT_LDADD) $(GDAL_LDADD) $(OM_LDADD)

BUILT_SOURCES = $(plugin_UIHEADERS) $(plugin_UISOURCES) $(plugin_UIMOC) $(plugin_MOC)
##
##  For standalone executable
##

omgui_SOURCES = 	main.cpp \
			openmodellergui.cpp \
			openmodellergui.h \
			request_file.cpp \
			occurrences_file.cpp \
			imagewriter.cpp \
			$(plugin_UISOURCES) \
			$(plugin_UIHEADERS) \
			$(plugin_UIMOC)

nodist_omgui_SOURCES = openmodellergui.moc.cpp	

omgui_CXXFLAGS = $(CXXFLAGS) $(EXTRA_CXXFLAGS) $(QT_CXXFLAGS) $(DEBUG_QGIS) $(GDAL_CFLAGS) $(OM_CXXFLAGS) -DQGISDATAPATH=\"$(QGISDATAPATH)\"
omgui_LDADD = $(QT_LDADD) $(GDAL_LDADD) $(OM_LDADD)

CLEANFILES = $(BUILT_SOURCES)

coordinatedir = $(QGISDATAPATH)
coordinate_DATA = wkt_defs.txt

EXTRA_DIST = $(plugin_UI) $(coordinate_DATA) icon_om.xpm





