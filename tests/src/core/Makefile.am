# Copyright (C) 2003 Gary Sherman <sherman at mrcc.com>
#  
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without 
# modifications, as long as this notice is preserved.
# 
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

bin_PROGRAMS = testqgsapplication \
		testqgsgeometry \
		testqgsfeature \
		testqgsfield \
               testqgssymbol

#
# Define some global variables that will be used for building each test
#

GLOBALLDADD = $(QT_LDADD) $(PG_LIB) $(GDAL_LDADD) -lproj ../../../src/core/libqgis_core.la ../../../src/gui/libqgis_gui.la  
GLOBALCXXFLAGS = $(CXXFLAGS) $(EXTRA_CXXFLAGS) $(GDAL_CFLAGS) $(QT_CXXFLAGS) $(PG_INC)  -I../../../src/core

#
# Instruction for running the qt4 meta object compiler 
#

%.moc.cpp: %.cpp
	$(MOC) -o $@ $<

BUILT_SOURCES = $(testqgsapplication_MOC) \
		$(testqgsgeometry_MOC) \
		$(testqgsfeature_MOC) \
		$(testqgsfield_MOC) \
                $(testqgssymbol_MOC)

CLEANFILES = $(BUILT_SOURCES)

#
# Specify the compilation files for each unit test now
#

testqgsapplication_MOC = testqgsapplication.moc.cpp
testqgsapplication_SOURCES = testqgsapplication.cpp 
testqgsapplication_LDADD = $(GLOBALLDADD)
testqgsapplication_CXXFLAGS =  $(GLOBALCXXFLAGS)


testqgssymbol_MOC = testqgssymbol.moc.cpp
testqgssymbol_SOURCES = testqgssymbol.cpp 
testqgssymbol_LDADD = $(GLOBALLDADD)
testqgssymbol_CXXFLAGS =  $(GLOBALCXXFLAGS)

testqgsfield_MOC = testqgsfield.moc.cpp
testqgsfield_SOURCES = testqgsfield.cpp
testqgsfield_LDADD = $(GLOBALLDADD)
testqgsfield_CXXFLAGS =  $(GLOBALCXXFLAGS)

testqgsfeature_MOC = testqgsfeature.moc.cpp
testqgsfeature_SOURCES = testqgsfeature.cpp
testqgsfeature_LDADD = $(GLOBALLDADD)
testqgsfeature_CXXFLAGS =  $(GLOBALCXXFLAGS)

testqgsgeometry_MOC = testqgsgeometry.moc.cpp
testqgsgeometry_SOURCES = testqgsgeometry.cpp
testqgsgeometry_LDADD = $(GLOBALLDADD)
testqgsgeometry_CXXFLAGS =  $(GLOBALCXXFLAGS)
