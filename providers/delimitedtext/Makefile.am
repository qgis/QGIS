## $Id$

plugindir = ${pkglibdir}
plugin_LTLIBRARIES = delimitedtextprovider.la 

delimitedtextprovider_la_SOURCES = qgsdelimitedtextprovider.cpp		\
				   ../../src/qgsfeature.cpp		\
				   ../../src/qgsfeatureattribute.cpp	\
				   ../../src/qgsrect.cpp		\
				   ../../src/qgspoint.cpp		\
				   ../../src/qgsfield.cpp

AM_CXXFLAGS := $(CXXFLAGS) $(GDAL_CFLAGS) $(EXTRA_CXXFLAGS) $(QT_CXXFLAGS) $(DEBUG_QGIS)

delimitedtextprovider_la_LIBADD = $(QT_LDADD)  $(GDAL_LIB)
delimitedtextprovider_la_LDFLAGS = -avoid-version -module

EXTRA_DIST = qgsdelimitedtextprovider.h