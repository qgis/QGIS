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

%.h: %.ui 
	$(UIC) -o $@ $<
if HAVE_QT4
	../../tools/qgis-qt3to4.pl -uic $@
endif

if !NO_UIC_IMPLEMENTATIONS
%.cpp: %.ui
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


plugin_UI = qgsgrassselectbase.h \
	    qgsgrassselectbase.cpp \
	    qgsgrasseditbase.h \
	    qgsgrasseditbase.cpp \
	    qgsgrasstoolsbase.h \
	    qgsgrasstoolsbase.cpp \
	    qgsgrassmapcalcbase.h \
	    qgsgrassmapcalcbase.cpp \
	    qgsgrassmodulebase.h \
	    qgsgrassmodulebase.cpp \
	    qgsgrassnewmapsetbase.h \
	    qgsgrassnewmapsetbase.cpp \
	    qgsgrassattributesbase.h \
	    qgsgrassattributesbase.cpp \
	    qgsgrassregionbase.h \
	    qgsgrassregionbase.cpp 

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
qgsgrassselectbase.cpp: qgsgrassselectbase.h qgsgrassselectbase.ui 
qgsgrassselectbase.h: qgsgrassselectbase.ui
qgsgrasseditbase.cpp: qgsgrasseditbase.h qgsgrasseditbase.ui
qgsgrasseditbase.h: qgsgrasseditbase.ui
qgsgrasstoolsbase.cpp: qgsgrasstoolsbase.h qgsgrasstoolsbase.ui
qgsgrasstoolsbase.h: qgsgrasstoolsbase.ui
qgsgrassmapcalcbase.cpp: qgsgrassmapcalcbase.h qgsgrassmapcalcbase.ui
qgsgrassmapcalcbase.h: qgsgrassmapcalcbase.ui
qgsgrassmodulebase.cpp: qgsgrassmodulebase.h qgsgrassmodulebase.ui
qgsgrassmodulebase.h: qgsgrassmodulebase.ui
qgsgrassnewmapsetbase.cpp: qgsgrassnewmapsetbase.h qgsgrassnewmapsetbase.ui
qgsgrassnewmapsetbase.h: qgsgrassnewmapsetbase.ui
qgsgrassattributesbase.cpp: qgsgrassattributesbase.h qgsgrassattributesbase.ui
qgsgrassattributesbase.h: qgsgrassattributesbase.ui
qgsgrassregionbase.cpp: qgsgrassregionbase.h qgsgrassregionbase.ui
qgsgrassregionbase.h: qgsgrassregionbase.ui

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
