if HAVE_QTMAC
MAC = mac
else 
PREFIX=-DPREFIX=\"$(prefix)\"
PLUGINPATH=-DPLUGINPATH=\"$(pkglibdir)\"
PKGDATAPATH=-DPKGDATAPATH=\"$(pkgdatadir)\"
endif

%.moc.cpp: %.h
	$(MOC) -o $@ $<
lib_LTLIBRARIES = libqgis_raster.la
libqgis_raster_la_MOC =	\
		qgsrasterlayer.moc.cpp
libqgis_raster_la_SOURCES = qgsrasterbandstats.h \
	qgsrasterpyramid.h \
	qgsrasterlayer.cpp \
	qgsrasterlayer.h \
	qgsrasterrendererif.h \
	qgsrasterviewport.h \
	$(libqgis_raster_la_MOC) 
#	qgsrastergrayscalerenderer.h \
#	qgsrastergrayscalerenderer.cpp 
BUILT_SOURCES = $(libqgis_raster_la_MOC) $(libqgis_raster_la_SOURCES)
##libqgis_raster_la_LDFLAGS = -no-undefined
libqgis_raster_la_CPPFLAGS = -I$(OMSRCDIR)
libqgis_raster_la_LIBADD = $(QT_LDADD) $(GDAL_LDADD)
libqgis_raster_la_CXXFLAGS = $(PREFIX) $(PKGDATAPATH) $(GDAL_CFLAGS) $(CXXFLAGS) $(EXTRA_CXXFLAGS) $(QT_CXXFLAGS) $(DEBUG_QGIS) -I../../widgets/projectionselector/ -I../gui/ -I../legend/ -I../ui/ -I../core/

EXTRA_DIST = \
	qgsrasterbandstats.h \
	qgsrasterpyramid.h 

CLEANFILES = $(libqgis_raster_la_MOC)
