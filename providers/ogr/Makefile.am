INCLUDES = -I../../src/core/ -I../../src

plugindir = ${pkglibdir}
plugin_LTLIBRARIES = ogrprovider.la 

ogrprovider_la_SOURCES = qgsogrprovider.cpp			\
			 qgsogrprovider.h

ogrprovider_la_LIBADD = $(QT_LDADD)  $(GDAL_LDADD) $(GEOS_LDADD) ../../src/core/libqgis_core.la ../../src/libqgis_gui.la
ogrprovider_la_LDFLAGS = -avoid-version -module
ogrprovider_la_CXXFLAGS := $(GDAL_CFLAGS) $(EXTRA_CXXFLAGS) $(QT_CXXFLAGS) $(DEBUG_QGIS) $(GEOS_CFLAGS)

