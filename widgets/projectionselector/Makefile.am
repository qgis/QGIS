# Copyright (C) 2005 Tim Sutton <tim at linfiniti.com>
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
PKGDATAPATH=-DPKGDATAPATH=\"$(pkgdatadir)\"


%.moc.cpp: %.h
	$(MOC) -o $@ $<

%.h: %.ui
	$(UIC) -o $@ $<

%.cpp: %.ui
	$(UIC) -o $@ -impl $*.h $<


# name of the qgis library widget
plugin_LTLIBRARIES = libqgsprojectionselector.la

plugin_MOC = qgsprojectionselector.moc.cpp		\
	     qgsprojectionselectorbase.moc.cpp		
	     

plugin_UI = qgsprojectionselectorbase.h \
	    qgsprojectionselectorbase.cpp 

plugin_UIC = qgsprojectionselectorbase.ui

libqgsprojectionselector_la_SOURCES =  qgsprojectionselector.cpp	\
			         $(plugin_UI)			\
			         $(plugin_MOC)

BUILT_SOURCES = $(plugin_MOC) $(plugin_UI)



libqgsprojectionselector_la_LIBADD =  ../../src/libqgis.la $(LDADD) $(QT_LDADD) $(GDAL_LDADD) -lsqlite3
libqgsprojectionselector_la_LDFLAGS = -avoid-version 
libqgsprojectionselector_la_CXXFLAGS = $(PKGDATAPATH) $(CXXFLAGS) $(GDAL_CFLAGS) $(EXTRA_CXXFLAGS) $(QT_CXXFLAGS) $(DEBUG_QGIS) -I../../src/

EXTRA_DIST = $(plugin_UIC) 
