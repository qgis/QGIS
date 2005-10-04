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

grassdatadir = ${pkgdatadir}/grass/
grassdata_DATA = *.gml

%.moc.cpp: %.h
	$(MOC) -o $@ $<

%.uic.h: %.ui
	$(UIC) -o $@ $<

%.h: %.ui 
	$(UIC) -o $@ $<
if HAVE_QT4
	../../tools/qgis-qt3to4.pl -uic $@
endif

if !NO_UIC_IMPLEMENTATIONS
%.uic.cpp: %.ui
	$(UIC) -o $@ -impl $*.h $<
endif

# %.h: %.ui
# 	$(UIC) -o $@ $<
#
# %.cpp: %.ui
# 	$(UIC) -o $@ -impl $*.h $<

plugin_LTLIBRARIES = grassplugin.la

plugin_MOC = qgsgrassplugin.moc.cpp \
	     qgsgrassselectbase.moc.cpp \
	     qgsgrassselect.moc.cpp \
	     qgsgrasseditbase.moc.cpp \
	     qgsgrassedit.moc.cpp \
	     qgsgrasstoolsbase.moc.cpp \
	     qgsgrasstools.moc.cpp \
	     qgsgrassmapcalcbase.moc.cpp \
	     qgsgrassmapcalc.moc.cpp \
	     qgsgrassmodulebase.moc.cpp \
	     qgsgrassmodule.moc.cpp \
	     qgsgrassnewmapsetbase.moc.cpp \
	     qgsgrassnewmapset.moc.cpp \
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
	    qgsgrassmapcalcbase.uic.h \
	    qgsgrassmapcalcbase.uic.cpp \
	    qgsgrassmodulebase.uic.h \
	    qgsgrassmodulebase.uic.cpp \
	    qgsgrassnewmapsetbase.uic.h \
	    qgsgrassnewmapsetbase.uic.cpp \
	    qgsgrassattributesbase.uic.h \
	    qgsgrassattributesbase.uic.cpp \
	    qgsgrassregionbase.uic.h \
	    qgsgrassregionbase.uic.cpp 

plugin_UIC = qgsgrassselectbase.ui \
	     qgsgrassselectbase.ui.h \
	     qgsgrasseditbase.ui \
	     qgsgrasseditbase.ui.h \
	     qgsgrasstoolsbase.ui \
	     qgsgrassmapcalcbase.ui \
	     qgsgrassmodulebase.ui \
	     qgsgrassnewmapsetbase.ui \
	     qgsgrassnewmapsetbase.ui.h \
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
			 qgsgrassmapcalc.cpp \
			 qgsgrassmapcalc.h \
			 qgsgrassmodule.cpp \
			 qgsgrassmodule.h \
			 qgsgrassnewmapset.cpp \
			 qgsgrassnewmapset.h \
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
qgsgrassmapcalcbase.uic.cpp: qgsgrassmapcalcbase.uic.h qgsgrassmapcalcbase.ui
qgsgrassmapcalcbase.uic.h: qgsgrassmapcalcbase.ui
qgsgrassmodulebase.uic.cpp: qgsgrassmodulebase.uic.h qgsgrassmodulebase.ui
qgsgrassmodulebase.uic.h: qgsgrassmodulebase.ui
qgsgrassnewmapsetbase.uic.cpp: qgsgrassnewmapsetbase.uic.h qgsgrassnewmapsetbase.ui
qgsgrassnewmapsetbase.uic.h: qgsgrassnewmapsetbase.ui
qgsgrassattributesbase.uic.cpp: qgsgrassattributesbase.uic.h qgsgrassattributesbase.ui
qgsgrassattributesbase.uic.h: qgsgrassattributesbase.ui
qgsgrassregionbase.uic.cpp: qgsgrassregionbase.uic.h qgsgrassregionbase.ui
qgsgrassregionbase.uic.h: qgsgrassregionbase.ui

BUILT_SOURCES = $(plugin_MOC) $(plugin_UI)


grassplugin_la_LIBADD = $(QT_LDADD) ../../providers/grass/libqgisgrass.la $(GRASS_LIB) $(PG_LIB) $(GDAL_LDADD)
grassplugin_la_LDFLAGS = -avoid-version -module
grassplugin_la_CXXFLAGS = $(PREFIX) $(CXXFLAGS) -I$(GRASS_INC) $(DEFS) $(EXTRA_CXXFLAGS) $(QT_CXXFLAGS) $(DEBUG_QGIS) -I$(PG_INC)  $(GDAL_CFLAGS) $(GEOS_CFLAGS) -I../../src/legend/

CLEANFILES = $(BUILT_SOURCES)

EXTRA_DIST = $(plugin_UIC) \
	add_vector.xpm \
	add_raster.xpm \
	grass_edit.xpm \
	grass_region.xpm \
	grass_region_edit.xpm \
	grass_tools.xpm \
	$(grassdata_DATA)
