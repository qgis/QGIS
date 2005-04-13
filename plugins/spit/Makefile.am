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

%.h: %.ui
	$(UIC) -o $@ $<

%.cpp: %.ui
	$(UIC) -o $@ -impl $*.h $<

spit_MOC = qgsspit.moc.cpp \
	qgsspitplugin.moc.cpp \
	qgsconnectiondialog.moc.cpp \
	qgsshapefile.moc.cpp \
	qgsmessageviewer.moc.cpp \
	qgsconnectiondialogbase.moc.cpp \
	qgsspitbase.moc.cpp \
	qgsmessageviewerbase.moc.cpp \
	qgseditreservedwordsbase.moc.cpp \
	qgseditreservedwordsdialog.moc.cpp \
	qgsscangeometries.moc.cpp

spit_UI = qgsconnectiondialogbase.h \
	qgsconnectiondialogbase.cpp \
	qgsspitbase.h \
	qgsspitbase.cpp \
	qgsmessageviewerbase.h \
	qgsmessageviewerbase.cpp \
	qgseditreservedwordsbase.cpp \
	qgseditreservedwordsbase.h \
	qgsscangeometries.h \
	qgsscangeometries.ui.h \
	qgsscangeometries.cpp

spit_UIC = qgsconnectiondialogbase.ui \
	qgsmessageviewerbase.ui \
	qgsspitbase.ui \
	qgseditreservedwordsbase.ui \
	qgsscangeometries.ui


spitplugin_la_SOURCES =  qgsspit.cpp \
	qgsspit.h \
	qgsspitplugin.cpp \
	qgsspitplugin.h \
	qgsdbfbase.h \
	qgsconnectiondialog.cpp \
	qgsconnectiondialog.h \
	qgsshapefile.cpp \
	qgsshapefile.h \
	qgsmessageviewer.cpp \
	qgsmessageviewer.h \
	qgseditreservedwordsdialog.h \
	qgseditreservedwordsdialog.cpp \
	$(spit_UI)

spit_SOURCES = $(spitplugin_la_SOURCES) ../../src/qgspgutil.cpp  main.cpp $(SPIT_UIC) $(spit_MOC)

nodist_spitplugin_la_SOURCES = $(spit_MOC)

# UI dependencies			
qgsconnectiondialogbase.cpp: qgsconnectiondialogbase.ui qgsconnectiondialogbase.h
qgsconnectiondialogbase.h: qgsconnectiondialogbase.ui
qgsspitbase.cpp: qgsspitbase.h qgsspitbase.ui
qgsspitbase.h: qgsspitbase.ui
qgsmessageviewerbase.cpp: qgsmessageviewerbase.h qgsmessageviewerbase.ui
qgsmessageviewerbase.h: qgsmessageviewerbase.ui

BUILT_SOURCES = $(spit_MOC) $(spit_UI)


spitplugin_la_LIBADD = $(QT_LDADD)  $(PG_LIB) $(GDAL_LDADD) ../../src/libqgis.la
spitplugin_la_LDFLAGS = -avoid-version -module
spitplugin_la_CXXFLAGS = $(CXXFLAGS) $(EXTRA_CXXFLAGS) $(GDAL_CFLAGS) $(QT_CXXFLAGS) -I$(PG_INC)  -I../../src


spit_LDADD = $(QT_LDADD)  $(PG_LIB) $(GDAL_LDADD)
#spit_LDFLAGS = -avoid-version -module
spit_CXXFLAGS = $(CXXFLAGS) $(EXTRA_CXXFLAGS) $(GDAL_CFLAGS) $(QT_CXXFLAGS) -I$(PG_INC)  -I../../src

EXTRA_DIST = $(spit_UIC) icon_spit.xpm spiticon.xpm spit_icons.h
