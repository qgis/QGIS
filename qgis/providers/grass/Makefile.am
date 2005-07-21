##
## $Id$
##

INCLUDES = -I../../src

lib_LTLIBRARIES = libqgisgrass.la

plugindir = ${pkglibdir}
plugin_LTLIBRARIES = grassprovider.la

%.moc.cpp: %.h
	$(MOC) -o $@ $<

%.h: %.ui
	$(UIC) -o $@ $<

%.cpp: %.ui
	$(UIC) -o $@ -impl $*.h $<

grassprovider_la_SOURCES = provider.cpp \
			qgsgrassprovider.h

grassprovider_la_LIBADD = $(QT_LDADD) libqgisgrass.la
grassprovider_la_LDFLAGS = -avoid-version -module
grassprovider_la_CXXFLAGS = $(CXXFLAGS) $(EXTRA_CXXFLAGS) $(QT_CXXFLAGS) -I$(GRASS_INC) $(DEBUG_QGIS) $(GEOS_CFLAGS) $(GDAL_CFLAGS)

libqgisgrass_la_SOURCES = qgsgrassprovider.cpp \
			qgsgrassprovider.h \
			qgsgrass.cpp \
			qgsgrass.h \
			../../src/qgsfeature.cpp \
			../../src/qgsfeatureattribute.cpp \
			../../src/qgsrect.cpp \
			../../src/qgspoint.cpp \
			../../src/qgsfield.cpp

libqgisgrass_la_LIBADD = $(QT_LDADD) $(GRASS_LIB)
libqgisgrass_la_LDFLAGS = -version-info $(INTERFACE_VERSION)
libqgisgrass_la_CXXFLAGS = $(CXXFLAGS) $(EXTRA_CXXFLAGS) $(QT_CXXFLAGS) -I$(GRASS_INC) $(DEBUG_QGIS) $(GEOS_CFLAGS) $(GDAL_CFLAGS)
if HAVE_POSTGRESQL
libqgisgrass_la_CXXFLAGS += -I$(PG_INC)
endif
