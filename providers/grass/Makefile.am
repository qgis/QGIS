##
## $Id$
##

INCLUDES = -I../../src

#lib_LTLIBRARIES = libqgisgrass.la

plugindir = ${pkglibdir}
plugin_LTLIBRARIES = libgrassprovider.la 

%.moc.cpp: %.h
	$(MOC) -o $@ $<

%.h: %.ui
	$(UIC) -o $@ $<

%.cpp: %.ui
	$(UIC) -o $@ -impl $*.h $<

#libqgisgrass_la_SOURCES = qgsgrassprovider.cpp \
#	                qgsgrassprovider.h \
#			qgsgrass.cpp \
#                       qgsgrass.h \
#		        ../../src/qgsfeature.cpp \
#			../../src/qgsfeatureattribute.cpp \
#			../../src/qgsrect.cpp \
#			../../src/qgspoint.cpp \
#			../../src/qgsfield.cpp 

libgrassprovider_la_SOURCES = provider.cpp \
			qgsgrassprovider.cpp \
	                qgsgrassprovider.h \
			qgsgrass.cpp \
                        qgsgrass.h \
		        ../../src/qgsfeature.cpp \
			../../src/qgsfeatureattribute.cpp \
			../../src/qgsrect.cpp \
			../../src/qgspoint.cpp \
			../../src/qgsfield.cpp 

#libqgisgrass_la_LIBADD = $(QT_LDADD) $(GRASS_LIB)
#libqgisgrass_la_LDFLAGS = -version-info $(INTERFACE_VERSION)
#libqgisgrass_la_CXXFLAGS = $(CXXFLAGS) -I$(GRASS_INC) -I$(PG_INC) $(EXTRA_CXXFLAGS) $(QT_CXXFLAGS) $(DEBUG_QGIS) 

libgrassprovider_la_LIBADD = $(QT_LDADD) $(GRASS_LIB)  $(GDAL_LDADD)
libgrassprovider_la_LDFLAGS = -avoid-version -module
libgrassprovider_la_CXXFLAGS = -I$(GRASS_INC) $(GDAL_CFLAGS) -I$(PG_INC) $(EXTRA_CXXFLAGS) $(QT_CXXFLAGS) $(DEBUG_QGIS) $(GEOS_CFLAGS)
