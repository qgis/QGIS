plugindir = ${pkglibdir}
plugin_LTLIBRARIES = ogrprovider.la 

ogrprovider_la_SOURCES = qgsshapefileprovider.cpp \
                      	qgsshapefileprovider.h \
												../../src/qgsfeature.cpp \
												../../src/qgsfeatureattribute.cpp \
												../../src/qgsrect.cpp \
												../../src/qgspoint.cpp \
												../../src/qgsfield.cpp


ogrprovider_la_LIBADD = $(QT_LDADD)  $(GDAL_LDADD) -lgeos
ogrprovider_la_LDFLAGS = -avoid-version -module
ogrprovider_la_CXXFLAGS := $(CXXFLAGS) $(GDAL_CFLAGS) $(EXTRA_CXXFLAGS) $(QT_CXXFLAGS) $(DEBUG_QGIS)
