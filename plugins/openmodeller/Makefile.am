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
UI = openmodellerguibase.ui omguimainbase.ui layerselectorbase.ui omguireportbase.ui

## uic created headers
UIHEADERS = $(UI:.ui=.uic.h)

## uic created source files
UISOURCES = $(UI:.ui=.uic.cpp)

## uic created MOC files
UIMOC = $(UI:.ui=.moc.uic.cpp)

## MOC files
plugin_MOC = plugin.moc.cpp openmodellergui.moc.cpp layerselector.moc.cpp


libopenmodellerplugin_la_SOURCES = plugin.cpp \
	plugin.h \
	qlogcallback.h \
	omggdal.cpp \
	openmodellergui.cpp \
	openmodellergui.h \
	request_file.cpp \
	occurrences_file.cpp \
	file_parser.cpp \
	imagewriter.cpp \
	layerselector.cpp \
	layerselector.h \
	$(UISOURCES) \
	$(UIHEADERS) \
	$(UIMOC)

nodist_libopenmodellerplugin_la_SOURCES = $(plugin_MOC)

libopenmodellerplugin_la_CXXFLAGS := $(CXXFLAGS) $(EXTRA_CXXFLAGS) $(QT_CXXFLAGS) $(QGIS_CXXFLAGS) $(GDAL_CFLAGS) $(OM_CXXFLAGS) -DQGISDATAPATH=\"$(QGISDATAPATH)\"
libopenmodellerplugin_la_LDFLAGS = -avoid-version -module $(QGIS_LDADD) $(QT_LDADD) $(GDAL_LDADD) $(OM_LDADD)

BUILT_SOURCES = $(UIHEADERS) $(UISOURCES) $(UIMOC) $(plugin_MOC)

##
##  For standalone executable
##

omgui_SOURCES = 	main.cpp \
			qlogcallback.h \
			omguimain.cpp \
			omggdal.cpp \
			openmodellergui.cpp \
			openmodellergui.h \
			request_file.cpp \
			file_parser.cpp \
			layerselector.cpp \
			layerselector.h \
			occurrences_file.cpp \
			imagewriter.cpp \
			$(UISOURCES) \
			$(UIHEADERS) \
			$(UIMOC)

nodist_omgui_SOURCES = openmodellergui.moc.cpp omguimain.moc.cpp layerselector.moc.cpp

omgui_CXXFLAGS = $(CXXFLAGS) $(EXTRA_CXXFLAGS) $(QT_CXXFLAGS) $(DEBUG_QGIS) $(GDAL_CFLAGS) $(OM_CXXFLAGS) -DQGISDATAPATH=\"$(QGISDATAPATH)\"
omgui_LDADD = $(QT_LDADD) $(GDAL_LDADD) $(OM_LDADD)

CLEANFILES = $(BUILT_SOURCES)

coordinatedir = $(QGISDATAPATH)
coordinate_DATA = wkt_defs.txt

EXTRA_DIST = $(UI) $(coordinate_DATA) icon_om.xpm

