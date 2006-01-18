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

INCLUDES = -I../../gui -I../../core

DEFS=-DGRASS_BASE=\"$(GRASS_BASE)\" -DHAVE_OPENPTY=$(HAVE_OPENPTY)

if !HAVE_QTMAC
PREFIX=-DPREFIX=\"$(prefix)\"
endif

plugindir = ${pkglibdir}

grassdatadir = ${pkgdatadir}/grass/
grassdata_DATA = *.gml images/world.png

%.moc.cpp: %.h
	$(MOC) -o $@ $<

ui_%.h: %.ui 
	$(UIC) -o $@ $<

plugin_LTLIBRARIES = grassplugin.la

plugin_MOC = qgsgrassplugin.moc.cpp \
	     qgsgrassselect.moc.cpp \
	     qgsgrassedit.moc.cpp \
	     qgsgrasstools.moc.cpp \
	     qgsgrassmapcalc.moc.cpp \
	     qgsgrassmodule.moc.cpp \
	     qgsgrassnewmapset.moc.cpp \
	     qgsgrassshell.moc.cpp \
	     qgsgrassattributes.moc.cpp \
	     qgsgrassregion.moc.cpp


plugin_UI = ui_qgsgrassselectbase.h \
	    ui_qgsgrasseditbase.h \
	    ui_qgsgrasstoolsbase.h \
	    ui_qgsgrassmapcalcbase.h \
	    ui_qgsgrassmodulebase.h \
	    ui_qgsgrassnewmapsetbase.h \
	    ui_qgsgrassshellbase.h \
	    ui_qgsgrassattributesbase.h \
	    ui_qgsgrassregionbase.h 

plugin_UIC = qgsgrassselectbase.ui \
	     qgsgrassselectbase.ui.h \
	     qgsgrasseditbase.ui \
	     qgsgrasstoolsbase.ui \
	     qgsgrassmapcalcbase.ui \
	     qgsgrassmodulebase.ui \
	     qgsgrassnewmapsetbase.ui \
	     qgsgrassshellbase.ui \
	     qgsgrassattributesbase.ui \
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
			 qgsgrassshell.cpp \
			 qgsgrassshell.h \
			 qgsgrassattributes.cpp \
			 qgsgrassattributes.h \
			 qgsgrassregion.cpp \
			 qgsgrassregion.h \
			 ../../providers/grass/qgsgrass.h \
			 ../../providers/grass/qgsgrassprovider.h \
			 $(plugin_UI)

nodist_grassplugin_la_SOURCES = $(plugin_MOC)

# UI dependencies      
qgsgrassselectbase.cpp: ui_qgsgrassselectbase.h qgsgrassselectbase.ui 
qgsgrassselectbase.h: qgsgrassselectbase.ui
qgsgrasseditbase.cpp: ui_qgsgrasseditbase.h qgsgrasseditbase.ui
qgsgrasseditbase.h: qgsgrasseditbase.ui
qgsgrasstoolsbase.cpp: ui_qgsgrasstoolsbase.h qgsgrasstoolsbase.ui
qgsgrasstoolsbase.h: qgsgrasstoolsbase.ui
qgsgrassmapcalcbase.cpp: ui_qgsgrassmapcalcbase.h qgsgrassmapcalcbase.ui
qgsgrassmapcalcbase.h: qgsgrassmapcalcbase.ui
qgsgrassmodulebase.cpp: ui_qgsgrassmodulebase.h qgsgrassmodulebase.ui
qgsgrassmodulebase.h: qgsgrassmodulebase.ui
qgsgrassnewmapsetbase.cpp: ui_qgsgrassnewmapsetbase.h qgsgrassnewmapsetbase.ui
qgsgrassnewmapsetbase.h: qgsgrassnewmapsetbase.ui
qgsgrassshellbase.cpp: ui_qgsgrassshellbase.h qgsgrassshellbase.ui
qgsgrassshellbase.h: qgsgrassshellbase.ui
qgsgrassattributesbase.cpp: ui_qgsgrassattributesbase.h qgsgrassattributesbase.ui
qgsgrassattributesbase.h: qgsgrassattributesbase.ui
qgsgrassregionbase.cpp: ui_qgsgrassregionbase.h qgsgrassregionbase.ui
qgsgrassregionbase.h: qgsgrassregionbase.ui

BUILT_SOURCES = $(plugin_MOC) $(plugin_UI)


grassplugin_la_LIBADD = $(QT_LDADD) ../../providers/grass/libqgisgrass.la $(GRASS_LIB) $(PG_LIB) $(GDAL_LDADD) ../../gui/libqgis_gui.la ../../core/libqgis_core.la ../../raster/libqgis_raster.la
grassplugin_la_LDFLAGS = -avoid-version -module
grassplugin_la_CXXFLAGS = $(PREFIX) $(CXXFLAGS) -I$(GRASS_INC) $(DEFS) $(EXTRA_CXXFLAGS) $(QT_CXXFLAGS) $(DEBUG_QGIS) -I$(PG_INC)  $(GDAL_CFLAGS) $(GEOS_CFLAGS) -I../../legend/ -I../../raster -I../../ui -I../../widgets/projectionselector

CLEANFILES = $(BUILT_SOURCES)

EXTRA_DIST = $(plugin_UIC) \
	add_vector.xpm \
	add_raster.xpm \
	grass_edit.xpm \
	grass_region.xpm \
	grass_region_edit.xpm \
	grass_tools.xpm \
	$(grassdata_DATA)
