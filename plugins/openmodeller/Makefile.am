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
plugin_LTLIBRARIES = libopenmodellerplugin.la

#for standalon executable
#bin_PROGRAMS = openmodellergui 

%.moc.cpp: %.h
	$(MOC) -o $@ $<

%.h: %.ui
	$(UIC) -o $@ $<

%.cpp: %.ui
	$(UIC) -o $@ -impl $*.h $<

##
##  For plugin lib
##

libopenmodellerplugin_la_SOURCES =  plugin.cpp \
			      openmodellergui.cpp \
			      file_parser.hh \
			      list.hh \
			      $(plugin_UI)\
			      $(plugin_MOC)

	     
plugin_MOC = plugin.moc.cpp \
             openmodellergui.moc.cpp \
	     openmodellerguibase.moc.cpp \
	     ../../src/qgsmaplayer.moc.cpp \
	     ../../src/qgsvectorlayer.moc.cpp 

plugin_UI = openmodellerguibase.h \
	    openmodellerguibase.cpp 

plugin_UIC = openmodellerguibase.ui

# UI dependencies      
openmodellerguibase.cpp: openmodellerguibase.h openmodellerguibase.ui

BUILT_SOURCES = $(plugin_MOC) $(plugin_UI)

libopenmodellerplugin_la_LIBADD = $(QT_LDADD) ../../src/libqgis.la -L/usr/lib -lopenmodeller
libopenmodellerplugin_la_LDFLAGS = -avoid-version -module 
libopenmodellerplugin_la_CXXFLAGS = $(CXXFLAGS) $(EXTRA_CXXFLAGS) $(QT_CXXFLAGS) $(DEBUG_QGIS) -I../../src -I/usr/local/include -I/usr/local/include/openmodeller

##
##  For standalone executable
##

openmodellergui_SOURCES =        main.cpp \
			      openmodellergui.cpp \
			      openmodellerguibase.cpp \
			      $(standalone_MOC) 

openmodellergui_LDADD = $(QT_LDADD) 
openmodellergui_CXXFLAGS = $(CXXFLAGS) $(EXTRA_CXXFLAGS) $(QT_CXXFLAGS) $(DEBUG_QGIS) -I../../src -I/usr/local/include -I/usr/local/include/openmodeller

standalone_MOC = openmodellergui.moc.cpp \
	     openmodellerguibase.moc.cpp 

standalone_UI = openmodellerguibase.h \
	    openmodellerguibase.cpp 


## 
## Anything that must go in the tarball gets added here
##

EXTRA_DIST = $(plugin_UIC) icon.xpm \
openmodellerguibase.h \
openmodellerguibase.ui.h \
openmodellergui.h
