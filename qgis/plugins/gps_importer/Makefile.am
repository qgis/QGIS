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

#for plugin so
plugin_LTLIBRARIES = libgpsimporterplugin.la

#for standalon executable
bin_PROGRAMS = gpsimporter 

%.moc.cpp: %.h
	$(MOC) -o $@ $<

%.h: %.ui
	$(UIC) -o $@ $<

%.cpp: %.ui
	$(UIC) -o $@ -impl $*.h $<

##
##  For plugin lib
##

libgpsimporterplugin_la_SOURCES =  plugin.cpp \
»       »       »             waypointtoshape.cpp \
                              waypointtoshape.h \
»       »       »             utils.c \
                              shpopen.c \
			      dbfopen.c \
			      shapefile.h \
			      plugingui.cpp \
			      $(plugin_UI)\
			      $(plugin_MOC)

	     
plugin_MOC = plugin.moc.cpp \
             plugingui.moc.cpp \
	     pluginguibase.moc.cpp \
	     ../../src/qgsmaplayer.moc.cpp \
	     ../../src/qgsvectorlayer.moc.cpp 

plugin_UI = pluginguibase.h \
	    pluginguibase.cpp 

plugin_UIC = pluginguibase.ui

# UI dependencies      
pluginguibase.cpp: pluginguibase.h pluginguibase.ui

BUILT_SOURCES = $(plugin_MOC) $(plugin_UI)

AM_CXXFLAGS := $(CXXFLAGS) $(EXTRA_CXXFLAGS) $(QT_CXXFLAGS) $(DEBUG_QGIS) -I../../src

libgpsimporterplugin_la_LIBADD = $(QT_LDADD) ../../src/libqgis.la

libgpsimporterplugin_la_LDFLAGS = -avoid-version -module

##
##  For standalone executable
##

gpsimporter_SOURCES =         main.cpp \
»       »       »             waypointtoshape.cpp \
                              waypointtoshape.h \
»       »       »             utils.c \
                              shpopen.c \
			      dbfopen.c \
			      shapefile.h \
			      plugingui.cpp \
			      pluginguibase.cpp \
			      $(standalone_MOC) 

gpsimporter_LDADD = $(QT_LDADD) 

standalone_MOC = plugingui.moc.cpp \
	     pluginguibase.moc.cpp 

standalone_UI = pluginguibase.h \
	    pluginguibase.cpp 


## 
## Anything that must go in the tarball gets added here
##

EXTRA_DIST = $(plugin_UIC) icon.xpm \
pluginguibase.h \
pluginguibase.ui.h \
plugingui.h \
plugin.h \
shapefil.h \
utils.h \
waypointtoshape.h
