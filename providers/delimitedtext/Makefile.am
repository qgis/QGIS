## $Id$

INCLUDES = -I../../src

plugindir = ${pkglibdir}
plugin_LTLIBRARIES = delimitedtextprovider.la 

delimitedtextprovider_la_SOURCES = qgsdelimitedtextprovider.cpp		\
				   ../../src/qgsfeature.cpp		\
				   ../../src/qgsfeatureattribute.cpp	\
				   ../../src/qgsrect.cpp		\
				   ../../src/qgspoint.cpp		\
				   ../../src/qgsfield.cpp


delimitedtextprovider_la_LIBADD = $(QT_LDADD)  $(GDAL_LIB)
delimitedtextprovider_la_LDFLAGS = -avoid-version -module
delimitedtextprovider_la_CXXFLAGS = $(GDAL_CFLAGS) $(EXTRA_CXXFLAGS) $(QT_CXXFLAGS) $(DEBUG_QGIS)

EXTRA_DIST = qgsdelimitedtextprovider.h
