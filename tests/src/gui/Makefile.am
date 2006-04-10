# Copyright (C) 2003 Gary Sherman <sherman at mrcc.com>
#  
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without 
# modifications, as long as this notice is preserved.
# 
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

bin_PROGRAMS = testqgisapp

#
# Define some global variables that will be used for building each test
#

GLOBALLDADD = $(QT_LDADD) \
				$(PG_LIB) \
				$(GDAL_LDADD) \
				-lproj \
				../../../src/core/libqgis_core.la \
				../../../src/raster/libqgis_raster.la \
				../../../src/gui/libqgis_gui.la  
GLOBALCXXFLAGS = $(CXXFLAGS) \
				$(EXTRA_CXXFLAGS) \
				$(GDAL_CFLAGS) \
				$(QT_CXXFLAGS) \
				$(PG_INC) \
				-I../../../src/raster \
				-I../../../src/core \
				-I../../../src/ui \
				-I../../../src/gui

#
# Instruction for running the qt4 meta object compiler 
#

%.moc.cpp: %.cpp
	$(MOC) -o $@ $<

BUILT_SOURCES = $(testqgisapp_MOC) 

CLEANFILES = $(BUILT_SOURCES)

#
# Specify the compilation files for each unit test now
#

testqgisapp_MOC = testqgisapp.moc.cpp
testqgisapp_SOURCES = testqgisapp.cpp
testqgisapp_LDADD = $(GLOBALLDADD)
testqgisapp_CXXFLAGS =  $(GLOBALCXXFLAGS)
