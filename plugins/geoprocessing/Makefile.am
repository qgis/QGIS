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
plugin_LTLIBRARIES = pggeoprocessingplugin.la

%.moc.cpp: %.h
	$(MOC) -o $@ $<

%.h: %.ui
	$(UIC) -o $@ $<

%.cpp: %.ui
	$(UIC) -o $@ -impl $*.h $<
	
pggeoprocessingplugin_la_SOURCES =  qgspggeoprocessing.cpp \
				qgspggeoprocessing.h \
        qgsdlgpgbuffer.cpp \
        qgsdlgpgbuffer.h \
				../../src/qgsmaplayer.cpp \
        ../../src/qgsvectorlayer.cpp \
        ../../src/qgsfield.cpp \
				$(pggeoprocessing_UI)
        
nodist_pggeoprocessingplugin_la_SOURCES = $(pggeoprocessing_MOC)
        
pggeoprocessing_MOC = 	qgspggeoprocessing.moc.cpp \
        ../../src/qgsmaplayer.moc.cpp \
        ../../src/qgsvectorlayer.moc.cpp \
        qgsdlgpgbufferbase.moc.cpp \
        qgsdlgpgbuffer.moc.cpp

pggeoprocessing_UI = qgsdlgpgbufferbase.h \
        qgsdlgpgbufferbase.cpp 
        
pggeoprocessing_UIC = qgsdlgpgbufferbase.ui

# UI dependencies			
qgsdlgpgbufferbase.cpp: qgsdlgpgbufferbase.h qgsdlgpgbufferbase.ui
qgsdlgpgbufferbase.h: qgsdlgpgbufferbase.ui

BUILT_SOURCES = $(pggeoprocessing_MOC) $(pggeoprocessing_UI)

AM_CXXFLAGS := $(CXXFLAGS) $(EXTRA_CXXFLAGS) $(QT_CXXFLAGS) $(PGDB) -I$(PG_INC) $(DEBUG_QGIS)

pggeoprocessingplugin_la_LIBADD = $(QT_LDADD)  $(PG_LIB)
pggeoprocessingplugin_la_LDFLAGS = -avoid-version -module

EXTRA_DIST = $(pggeoprocessing_UIC) icon_buffer.xpm

