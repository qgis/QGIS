lib_LTLIBRARIES = ogrprovider.la 

ogrprovider_la_SOURCES = qgsshapefileprovider.cpp \
                      ../../src/qgsfeature.cpp \
                       ../../src/qgsfeatureattribute.cpp \
                       ../../src/qgsrect.cpp \
                       ../../src/qgspoint.cpp \
                       ../../src/qgsfield.cpp

AM_CXXFLAGS := $(CXXFLAGS) $(GDAL_CFLAGS) $(EXTRA_CXXFLAGS) $(QT_CXXFLAGS) $(DEBUG_QGIS)

ogrprovider_la_LIBADD = $(QT_LDADD)  $(GDAL_LIB)
ogrprovider_la_LDFLAGS = -avoid-version -module
