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

if HAVE_QT3
%.uic.h: %.ui 
	$(UIC) -o $@ $<
endif

if HAVE_QT4
%.uic.h: %.ui 
	$(UIC) -o $@ $<
	../../tools/qgis-qt3to4.pl -uic $@
endif

if !NO_UIC_IMPLEMENTATIONS
%.uic.cpp: %.ui
	$(UIC) -o $@ -impl $*.uic.h $<
endif

# %.h: %.ui
# 	$(UIC) -o $@ $<
#
# %.cpp: %.ui
# 	$(UIC) -o $@ -impl $*.h $<

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


plugin_UI = qgsgrassselectbase.uic.h \
	    qgsgrassselectbase.uic.cpp \
	    qgsgrasseditbase.uic.h \
	    qgsgrasseditbase.uic.cpp \
	    qgsgrasstoolsbase.uic.h \
	    qgsgrasstoolsbase.uic.cpp \
	    qgsgrassmodulebase.uic.h \
	    qgsgrassmodulebase.uic.cpp \
	    qgsgrassattributesbase.uic.h \
	    qgsgrassattributesbase.uic.cpp \
	    qgsgrassregionbase.uic.h \
	    qgsgrassregionbase.uic.cpp 

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
qgsgrassselectbase.uic.cpp: qgsgrassselectbase.uic.h qgsgrassselectbase.ui 
qgsgrassselectbase.uic.h: qgsgrassselectbase.ui
qgsgrasseditbase.uic.cpp: qgsgrasseditbase.uic.h qgsgrasseditbase.ui
qgsgrasseditbase.uic.h: qgsgrasseditbase.ui
qgsgrasstoolsbase.uic.cpp: qgsgrasstoolsbase.uic.h qgsgrasstoolsbase.ui
qgsgrasstoolsbase.uic.h: qgsgrasstoolsbase.ui
qgsgrassmodulebase.uic.cpp: qgsgrassmodulebase.uic.h qgsgrassmodulebase.ui
qgsgrassmodulebase.uic.h: qgsgrassmodulebase.ui
qgsgrassattributesbase.uic.cpp: qgsgrassattributesbase.uic.h qgsgrassattributesbase.ui
qgsgrassattributesbase.uic.h: qgsgrassattributesbase.ui
qgsgrassregionbase.uic.cpp: qgsgrassregionbase.uic.h qgsgrassregionbase.ui
qgsgrassregionbase.uic.h: qgsgrassregionbase.ui

BUILT_SOURCES = $(plugin_MOC) $(plugin_UI)


grassplugin_la_LIBADD = $(QT_LDADD) ../../providers/grass/libqgisgrass.la $(GRASS_LIB) $(PG_LIB) $(GDAL_LDADD)
grassplugin_la_LDFLAGS = -avoid-version -module
grassplugin_la_CXXFLAGS = $(PREFIX) $(CXXFLAGS) -I$(GRASS_INC) $(DEFS) $(EXTRA_CXXFLAGS) $(QT_CXXFLAGS) $(DEBUG_QGIS) -I$(PG_INC)  $(GDAL_CFLAGS) $(GEOS_CFLAGS) -I../../src/legend/

EXTRA_DIST = $(plugin_UIC) \
	add_vector.xpm \
	add_raster.xpm \
	grass_edit.xpm \
	grass_region.xpm \
	grass_region_edit.xpm \
	grass_tools.xpm
