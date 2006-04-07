# Copyright (C) 2003 Gary Sherman <sherman at mrcc.com>
#  
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without 
# modifications, as long as this notice is preserved.
# 
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

bin_PROGRAMS = testqgsapplication


%.moc.cpp: %.h
	$(MOC) -o $@ $<

testqgsapplication_MOC = testqgsapplication.moc.cpp
testqgsapplication_SOURCES = testqgsapplication.cpp 

testqgsapplication_LDADD = $(QT_LDADD) $(PG_LIB) $(GDAL_LDADD) -lproj ../../../src/core/libqgis_core.la ../../../src/gui/libqgis_gui.la -lQtTest
testqgsapplication_CXXFLAGS = $(CXXFLAGS) $(EXTRA_CXXFLAGS) $(GDAL_CFLAGS) $(QT_CXXFLAGS) $(PG_INC)  -I../../../src/core

BUILT_SOURCES = $(testqgsapplication_MOC)

CLEANFILES = $(BUILT_SOURCES)
