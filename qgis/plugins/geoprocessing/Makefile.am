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

%.moc.cpp: %.h
	$(MOC) -o $@ $<

%.h: %.ui
	$(UIC) -o $@ $<

%.cpp: %.ui
	$(UIC) -o $@ -impl $*.h $<


plugin_LTLIBRARIES = pggeoprocessingplugin.la


pggeoprocessing_MOC = 	qgspggeoprocessing.moc.cpp \
		        qgsdlgpgbufferbase.moc.cpp \
		        qgsdlgpgbuffer.moc.cpp

pggeoprocessing_UI = qgsdlgpgbufferbase.h \
                     qgsdlgpgbufferbase.cpp 

pggeoprocessing_UIC = qgsdlgpgbufferbase.ui

pggeoprocessingplugin_la_SOURCES =  qgspggeoprocessing.cpp \
				    qgspggeoprocessing.h \
				    qgsdlgpgbuffer.cpp \
				    qgsdlgpgbuffer.h \
				    $(pggeoprocessing_UI)

nodist_pggeoprocessingplugin_la_SOURCES = $(pggeoprocessing_MOC)

# UI dependencies			
qgsdlgpgbufferbase.cpp: qgsdlgpgbufferbase.h qgsdlgpgbufferbase.ui
qgsdlgpgbufferbase.h: qgsdlgpgbufferbase.ui


BUILT_SOURCES = $(pggeoprocessing_MOC) $(pggeoprocessing_UI)


pggeoprocessingplugin_la_LIBADD = $(QT_LDADD)  $(GDAL_LDADD) $(PG_LIB) ../../src/libqgis.la
pggeoprocessingplugin_la_LDFLAGS = -avoid-version -module
pggeoprocessingplugin_la_CXXFLAGS = $(CXXFLAGS) $(EXTRA_CXXFLAGS) $(QT_CXXFLAGS) -I$(PG_INC) $(DEBUG_QGIS) -I../../src

EXTRA_DIST = $(pggeoprocessing_UIC) icon_buffer.xpm

