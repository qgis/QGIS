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

plugin_LTLIBRARIES = spitplugin.la
bin_PROGRAMS = spit


%.moc.cpp: %.h
	$(MOC) -o $@ $<

ui_%.h: %.ui 
	$(UIC) -o $@ $<

spit_MOC = qgsspit.moc.cpp \
	qgsshapefile.moc.cpp \
	qgseditreservedwordsdialog.moc.cpp 


spitplugin_la_MOC = $(spit_MOC) qgsspitplugin.moc.cpp

spit_UI = ui_qgsconnectiondialogbase.h \
	ui_qgsspitbase.h \
	ui_qgseditreservedwordsbase.h 

spit_UIC = qgsconnectiondialogbase.ui \
	qgsspitbase.ui \
	qgseditreservedwordsbase.ui 

common_sources =  qgsspit.cpp \
	qgsspit.h \
	qgsdbfbase.h \
	qgsconnectiondialog.cpp \
	qgsconnectiondialog.h \
	qgsshapefile.cpp \
	qgsshapefile.h \
	qgseditreservedwordsdialog.h \
	qgseditreservedwordsdialog.cpp \
	$(spit_UI)

spitplugin_la_SOURCES =  $(common_sources) \
	qgsspitplugin.cpp \
	qgsspitplugin.h

spit_SOURCES = ../../src/qgsmessageviewer.cpp ../../src/qgspgutil.cpp $(common_sources) main.cpp $(SPIT_UIC) $(spit_MOC)

nodist_spitplugin_la_SOURCES = $(spitplugin_la_MOC)

# UI dependencies			
##qgsconnectiondialogbase.cpp: qgsconnectiondialogbase.ui qgsconnectiondialogbase.h
##qgsconnectiondialogbase.h: qgsconnectiondialogbase.ui
##qgsspitbase.cpp: qgsspitbase.h qgsspitbase.ui
##qgsspitbase.h: qgsspitbase.ui

BUILT_SOURCES = $(spit_MOC) $(spit_UI) $(spitplugin_la_MOC)


spitplugin_la_LIBADD = $(QT_LDADD)  $(PG_LIB) $(GDAL_LDADD) ../../src/libqgis.la
spitplugin_la_LDFLAGS = -avoid-version -module
spitplugin_la_CXXFLAGS = $(CXXFLAGS) $(EXTRA_CXXFLAGS) $(GDAL_CFLAGS) $(QT_CXXFLAGS) $(PG_INC) $(GEOS_CFLAGS) -I../../src -I../../src/ui


spit_LDADD = $(QT_LDADD)  $(PG_LIB) $(GDAL_LDADD)
#spit_LDFLAGS = -avoid-version -module
spit_CXXFLAGS = $(CXXFLAGS) $(EXTRA_CXXFLAGS) $(GDAL_CFLAGS) $(QT_CXXFLAGS) $(PG_INC)  -I../../src -I../../src/ui

CLEANFILES = $(BUILT_SOURCES)

EXTRA_DIST = $(spit_UIC) icon_spit.xpm spiticon.xpm spit_icons.h
