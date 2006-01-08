## $Id$

INCLUDES = -I../../core/ -I../../gui/

plugindir = ${pkglibdir}
plugin_LTLIBRARIES = delimitedtextprovider.la 

delimitedtextprovider_la_SOURCES = qgsdelimitedtextprovider.cpp


delimitedtextprovider_la_LIBADD = $(QT_LDADD)  $(GDAL_LIB) $(GEOS_LDADD) ../../gui/libqgis_gui.la ../../core/libqgis_core.la
delimitedtextprovider_la_LDFLAGS = -avoid-version -module
delimitedtextprovider_la_CXXFLAGS = $(GDAL_CFLAGS) $(EXTRA_CXXFLAGS) $(QT_CXXFLAGS) $(DEBUG_QGIS) $(GEOS_CFLAGS)

EXTRA_DIST = qgsdelimitedtextprovider.h
