## $Id$

INCLUDES = -I../../src

plugindir = ${pkglibdir}
plugin_LTLIBRARIES = gpxprovider.la 

gpxprovider_la_SOURCES = qgsgpxprovider.cpp		\
                         gpsdata.cpp			\
			 ../../src/qgsfeature.cpp		\
			 ../../src/qgsfeatureattribute.cpp	\
			 ../../src/qgsrect.cpp		\
			 ../../src/qgspoint.cpp		\
			 ../../src/qgsfield.cpp


gpxprovider_la_LIBADD = $(QT_LDADD) $(GEOS_LDADD)
gpxprovider_la_LDFLAGS = -avoid-version -module
gpxprovider_la_CXXFLAGS = $(EXTRA_CXXFLAGS) $(QT_CXXFLAGS) $(DEBUG_QGIS) $(GEOS_CFLAGS)

EXTRA_DIST = qgsgpxprovider.h gpsdata.h
