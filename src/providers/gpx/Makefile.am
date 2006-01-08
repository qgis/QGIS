## $Id$

if HAVE_EXPAT

INCLUDES = -I../../core -I../../

plugindir = ${pkglibdir}
plugin_LTLIBRARIES = gpxprovider.la 

gpxprovider_la_SOURCES = qgsgpxprovider.cpp		\
	gpsdata.cpp


gpxprovider_la_LIBADD = $(QT_LDADD) $(GEOS_LDADD) ../../core/libqgis_core.la
gpxprovider_la_LDFLAGS = -avoid-version -module
gpxprovider_la_CXXFLAGS = $(EXTRA_CXXFLAGS) $(QT_CXXFLAGS) $(DEBUG_QGIS) $(GEOS_CFLAGS) $(GDAL_CFLAGS)

EXTRA_DIST = qgsgpxprovider.h gpsdata.h

endif
