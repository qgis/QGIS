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

plugindir = ${pkglibdir}/designer


%.moc.cpp: %.h
	$(MOC) -o $@ $<

# name of the designer plugin
plugin_LTLIBRARIES = qgsdesignerwidgets.la

plugin_MOC = qgsdesignerwidgets.moc.cpp 

qgsdesignerwidgets_la_SOURCES =  qgsdesignerwidgets.cpp \
			         $(plugin_MOC)
BUILT_SOURCES = $(plugin_MOC) 

qgsdesignerwidgets_la_LIBADD =  $(QT_LDADD) ../src/libqgis.la $(GDAL_LDADD)
qgsdesignerwidgets_la_LDFLAGS = -avoid-version -module
qgsdesignerwidgets_la_CXXFLAGS = $(CXXFLAGS) $(EXTRA_CXXFLAGS) $(QT_CXXFLAGS) $(DEBUG_QGIS) -I../src/

