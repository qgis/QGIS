# Copyright (C) 2004 Radim Blazek
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

SUBDIRS = config modules themes

INCLUDES = -I../../src

DEFS=-DGRASS_BASE=\"$(GRASS_BASE)\"
if !HAVE_QTMAC
PREFIX=-DPREFIX=\"$(prefix)\"
endif

plugindir = ${pkglibdir}

%.moc.cpp: %.h
	$(MOC) -o $@ $<

%.h: %.ui
	$(UIC) -o $@ $<

%.cpp: %.ui
	$(UIC) -o $@ -impl $*.h $<

plugin_LTLIBRARIES = grassplugin.la

plugin_MOC = qgsgrassplugin.moc.cpp \
	     qgsgrassselectbase.moc.cpp \
	     qgsgrasseditbase.moc.cpp \
	     qgsgrassedit.moc.cpp \
	     qgsgrasstoolsbase.moc.cpp \
	     qgsgrasstools.moc.cpp \
	     qgsgrassmodulebase.moc.cpp \
	     qgsgrassmodule.moc.cpp \
	     qgsgrassattributesbase.moc.cpp \
	     qgsgrassattributes.moc.cpp \
	     qgsgrassregionbase.moc.cpp \
	     qgsgrassregion.moc.cpp


plugin_UI = qgsgrassselectbase.h \
	    qgsgrassselectbase.cpp \
	    qgsgrasseditbase.h \
	    qgsgrasseditbase.cpp \
	    qgsgrasstoolsbase.h \
	    qgsgrasstoolsbase.cpp \
	    qgsgrassmodulebase.h \
	    qgsgrassmodulebase.cpp \
	    qgsgrassattributesbase.h \
	    qgsgrassattributesbase.cpp \
	    qgsgrassregionbase.h \
	    qgsgrassregionbase.cpp 

plugin_UIC = qgsgrassselectbase.ui \
	     qgsgrassselectbase.ui.h \
	     qgsgrasseditbase.ui \
	     qgsgrasseditbase.ui.h \
	     qgsgrasstoolsbase.ui \
	     qgsgrassmodulebase.ui \
	     qgsgrassattributesbase.ui \
	     qgsgrassattributesbase.ui.h \
	     qgsgrassregionbase.ui 

grassplugin_la_SOURCES = qgsgrassplugin.cpp \
			 qgsgrassplugin.h \
			 qgsgrassselect.cpp \
			 qgsgrassselect.h \
			 qgsgrassedit.cpp \
			 qgsgrassedit.h \
			 qgsgrasstools.cpp \
			 qgsgrasstools.h \
			 qgsgrassmodule.cpp \
			 qgsgrassmodule.h \
			 qgsgrassattributes.cpp \
			 qgsgrassattributes.h \
			 qgsgrassregion.cpp \
			 qgsgrassregion.h \
			 ../../providers/grass/qgsgrass.h \
			 ../../providers/grass/qgsgrassprovider.h \
			 $(plugin_UI) \
			 ../../src/qgssymbol.cpp

nodist_grassplugin_la_SOURCES = $(plugin_MOC)

# UI dependencies      
qgsgrassselectbase.cpp: qgsgrassselectbase.h qgsgrassselectbase.ui 
qgsgrassselectbase.h: qgsgrassselectbase.ui
qgsgrasseditbase.cpp: qgsgrasseditbase.h qgsgrasseditbase.ui
qgsgrasseditbase.h: qgsgrasseditbase.ui
qgsgrasstoolsbase.cpp: qgsgrasstoolsbase.h qgsgrasstoolsbase.ui
qgsgrasstoolsbase.h: qgsgrasstoolsbase.ui
qgsgrassmodulebase.cpp: qgsgrassmodulebase.h qgsgrassmodulebase.ui
qgsgrassmodulebase.h: qgsgrassmodulebase.ui
qgsgrassattributesbase.cpp: qgsgrassattributesbase.h qgsgrassattributesbase.ui
qgsgrassattributesbase.h: qgsgrassattributesbase.ui
qgsgrassregionbase.cpp: qgsgrassregionbase.h qgsgrassregionbase.ui
qgsgrassregionbase.h: qgsgrassregionbase.ui

BUILT_SOURCES = $(plugin_MOC) $(plugin_UI)


grassplugin_la_LIBADD = $(QT_LDADD) ../../providers/grass/libqgisgrass.la $(GRASS_LIB) $(PG_LIB) $(GDAL_LDADD)
grassplugin_la_LDFLAGS = -avoid-version -module
grassplugin_la_CXXFLAGS = $(PREFIX) $(CXXFLAGS) -I$(GRASS_INC) $(DEFS) $(EXTRA_CXXFLAGS) $(QT_CXXFLAGS) $(DEBUG_QGIS) -I$(PG_INC)  $(GDAL_CFLAGS) $(GEOS_CFLAGS)

EXTRA_DIST = $(plugin_UIC) \
	add_vector.xpm \
	add_raster.xpm \
	grass_edit.xpm \
	grass_region.xpm \
	grass_region_edit.xpm \
	grass_tools.xpm
