# Copyright (C) 2003 Gary Sherman <sherman at mrcc.com>
#  
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without 
# modifications, as long as this notice is preserved.
# 
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

plugindir = ${pkglibdir}
plugin_LTLIBRARIES = maplayerplugin.la

%.moc.cpp: %.h
	$(MOC) -o $@ $<

%.h: %.ui
	$(UIC) -o $@ $<

%.cpp: %.ui
	$(UIC) -o $@ -impl $*.h $<

maplayer_MOC = 	maplayertest.moc.cpp

maplayer_UI = 

maplayerplugin_la_SOURCES =  maplayertest.cpp \
				$(maplayer_UI)\
				$(maplayer_MOC)

# UI dependencies			


maplayerplugin_la_LIBADD = $(QT_LDADD) ../../src/libqgis.la
maplayerplugin_la_LDFLAGS = -avoid-version -module
maplayerplugin_la_CXXFLAGS = $(CXXFLAGS) $(EXTRA_CXXFLAGS) $(QT_CXXFLAGS) -I../../src
