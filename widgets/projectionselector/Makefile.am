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

selectordir = ${pkglibdir}
if !HAVE_QTMAC
PKGDATAPATH=-DPKGDATAPATH=\"$(pkgdatadir)\"
endif

%.moc.cpp: %.h
	$(MOC) -o $@ $<

ui_%.h: %.ui
	$(UIC) -o $@ $<

# name of the qgis library widget
selector_LTLIBRARIES = libqgsprojectionselector.la

selector_MOC = qgsprojectionselector.moc.cpp		

selector_UIC = qgsprojectionselectorbase.ui 

selector_UI = ui_qgsprojectionselectorbase.h 


libqgsprojectionselector_la_SOURCES =  qgsprojectionselector.cpp \
			         $(selector_UI) \
			         $(selector_UIC) \
			         $(selector_MOC) 

BUILT_SOURCES = $(selector_MOC) $(selector_UI) 

CLEANFILES = $(BUILT_SOURCES)


libqgsprojectionselector_la_LIBADD =  ../../src/libqgis.la $(LDADD) $(QT_LDADD) $(GDAL_LDADD) -lsqlite3
libqgsprojectionselector_la_LDFLAGS = -avoid-version 
libqgsprojectionselector_la_CXXFLAGS = $(PKGDATAPATH) $(CXXFLAGS) $(GDAL_CFLAGS) $(EXTRA_CXXFLAGS) $(QT_CXXFLAGS) $(DEBUG_QGIS) -I../../src/

EXTRA_DIST = $(selector_UIC)
