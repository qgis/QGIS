lib_LTLIBRARIES     = libogrprovider.la 

libogrprovider_la_SOURCES = qgsshapefileprovider.cpp \
                      ../../src/qgsfeature.cpp \
                       ../../src/qgsfeatureattribute.cpp \
                       ../../src/qgsrect.cpp \
                       ../../src/qgspoint.cpp \
                       ../../src/qgsfield.cpp

AM_CXXFLAGS := $(CXXFLAGS) $(GDAL_CFLAGS) $(EXTRA_CXXFLAGS) $(QT_CXXFLAGS) $(DEBUG_QGIS)

libogrprovider_la_LIBADD = $(QT_LDADD)  $(GDAL_LIB)


