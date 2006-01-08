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
# $Id: Makefile.am 4490 2006-01-07 01:09:27Z timlinux $


plugindir = ${pkglibdir}/designer

%.moc.cpp: %.h
	$(MOC) -o $@ $<

# name of the designer plugin
plugin_LTLIBRARIES = qgisdesignerwidgets.la

qgisdesignerwidgets_la_SOURCES =  qgsdesignerwidgets.cpp \
		qgsdesignerwidgets.h \
		$(projectionselector_la_MOC)

qgisdesignerwidgets_la_MOC = qgsprojectionselectorplugin.moc.cpp

projectionselector_la_LIBADD =  ../src/libqgis.la $(QT_LDADD)  
#qgisdesignerwidgets_la_LIBADD =  $(QT_LDADD) ../widgets/projectionselector/libqgsprojectionselector.la
qgisdesignerwidgets_la_LDFLAGS = -avoid-version -module
qgisdesignerwidgets_la_CXXFLAGS = $(CXXFLAGS) $(EXTRA_CXXFLAGS) $(QT_CXXFLAGS) $(DEBUG_QGIS) -I../ -I../legend
