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

if !HAVE_QTMAC
PKGDATAPATH=-DPKGDATAPATH=\"$(pkgdatadir)\"
endif

plugindir = ${pkglibdir}

#for plugin so
plugin_LTLIBRARIES = gridmakerplugin.la

#for standalon executable
bin_PROGRAMS = gridmaker 

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

##
##  For plugin lib
##

gridmakerplugin_la_SOURCES = plugin.cpp \
	graticulecreator.cpp \
	graticulecreator.h \
	utils.c \
	shpopen.c \
	dbfopen.c \
	shapefile.h \
	plugingui.cpp \
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

gridmakerplugin_la_LIBADD = $(QT_LDADD) $(GDAL_LDADD) ../../src/libqgis.la
gridmakerplugin_la_CFLAGS = $(CFLAGS) $(EXTRA_CFLAGS) $(DEBUG_QGIS)
gridmakerplugin_la_CXXFLAGS = $(CXXFLAGS) $(EXTRA_CXXFLAGS) $(QT_CXXFLAGS) $(DEBUG_QGIS) $(GDAL_CFLAGS) $(GEOS_CFLAGS) -I../../src
gridmakerplugin_la_LDFLAGS = -avoid-version -module

##
##  For standalone executable
##

gridmaker_SOURCES = main.cpp \
	graticulecreator.cpp \
	graticulecreator.h \
	utils.c \
	shpopen.c \
	dbfopen.c \
	shapefile.h \
	plugingui.cpp \
	pluginguibase.uic.cpp \
	$(standalone_MOC) 

gridmaker_LDADD = $(QT_LDADD) 
gridmaker_CFLAGS = $(CFLAGS) $(EXTRA_CFLAGS) $(DEBUG_QGIS)
gridmaker_CXXFLAGS = $(CXXFLAGS) $(EXTRA_CXXFLAGS) $(QT_CXXFLAGS) $(PKGDATAPATH) $(DEBUG_QGIS) -I../../src 

standalone_MOC = plugingui.moc.cpp \
	     pluginguibase.moc.cpp 

standalone_UI = pluginguibase.uic.h \
	    pluginguibase.uic.cpp 

CLEANFILES = $(BUILT_SOURCES)

## 
## Anything that must go in the tarball gets added here
##

EXTRA_DIST = $(plugin_UIC) icon.xpm \
pluginguibase.uic.h \
pluginguibase.ui.h \
plugingui.h \
plugin.h \
utils.h
