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

plugindir = ${pkglibdir}

%.moc.cpp: %.h
	$(MOC) -o $@ $<

%.h: %.ui
	$(UIC) -o $@ $<

%.cpp: %.ui
	$(UIC) -o $@ -impl $*.h $<

plugin_LTLIBRARIES = grassplugin.la

plugin_MOC = qgsgrassplugin.moc.cpp \
	     qgsgrassselectbase.moc.cpp

plugin_UI = qgsgrassselectbase.h \
	     qgsgrassselectbase.cpp

plugin_UIC = qgsgrassselectbase.ui \
       qgsgrassselectbase.ui.h

grassplugin_la_SOURCES =  qgsgrassplugin.cpp \
			      qgsgrassplugin.h \
			      qgsgrassselect.cpp \
            qgsgrassselect.h \
			      ../../providers/grass/qgsgrass.cpp \
			      ../../providers/grass/qgsgrass.h \
			      $(plugin_UI)\
            ../../src/qgssymbol.cpp

nodist_grassplugin_la_SOURCES = $(plugin_MOC)

# UI dependencies      
qgsgrassselectbase.cpp: qgsgrassselectbase.h qgsgrassselectbase.ui
qgsgrassselectbase.h: qgsgrassselectbase.ui

BUILT_SOURCES = $(plugin_MOC) $(plugin_UI)


grassplugin_la_LIBADD = $(QT_LDADD) $(GRASS_LIB) $(PG_LIB)
grassplugin_la_LDFLAGS = -avoid-version -module
grassplugin_la_CXXFLAGS = $(CXXFLAGS) -I$(GRASS_INC) $(EXTRA_CXXFLAGS) $(QT_CXXFLAGS) $(DEBUG_QGIS) -I$(PG_INC) -I../../src/

EXTRA_DIST = $(plugin_UIC) add_vector.xpm
