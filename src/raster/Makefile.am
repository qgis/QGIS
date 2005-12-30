if HAVE_QTMAC
MAC = mac
else 
PREFIX=-DPREFIX=\"$(prefix)\"
PLUGINPATH=-DPLUGINPATH=\"$(pkglibdir)\"
PKGDATAPATH=-DPKGDATAPATH=\"$(pkgdatadir)\"
endif

%.moc.cpp: %.h
	$(MOC) -o $@ $<
lib_LTLIBRARIES = libqgsraster.la
libqgsraster_la_MOC =	\
		qgsrasterlayer.moc.cpp
libqgsraster_la_SOURCES = qgsrasterbandstats.h \
	qgsrasterpyramid.h \
	qgsrasterlayer.cpp \
	qgsrasterlayer.h \
	qgsrasterrendererif.h \
	qgsrasterviewport.h \
	$(libqgsraster_la_MOC)
	##qgsrastergrayscalerenderer.h \
	##qgsrastergrayscalerenderer.cpp 
BUILT_SOURCES = $(libqgsraster_la_MOC) $(libqgsraster_la_SOURCES)
libqgsraster_la_LDFLAGS = -no-undefined
libqgsraster_la_CPPFLAGS = -I$(OMSRCDIR)
libqgsraster_la_LIBADD = $(QT_LDADD) $(GDAL_LDADD)
libqgsraster_la_CXXFLAGS = $(PREFIX) $(PKGDATAPATH) $(GDAL_CFLAGS) $(CXXFLAGS) $(EXTRA_CXXFLAGS) $(QT_CXXFLAGS) $(DEBUG_QGIS) -I../../widgets/projectionselector/ -I../ -I../legend/

EXTRA_DIST = \
	qgsrasterbandstats.h \
	qgsrasterpyramid.h 
