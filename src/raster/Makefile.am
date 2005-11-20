EXTRA_DIST = \
	qgsrasterbandstats.h \
	qgsrasterpyramid.h 

lib_LTLIBRARIES = libqgsraster.la

libqgsraster_la_SOURCES = qgsrasterbandstats.h \
	qgsrasterpyramid.h 
libqgsraster_la_LDFLAGS = -no-undefined
libqgsraster_la_CPPFLAGS = -I$(OMSRCDIR)
libqgsraster_la_LIBADD = $(QT_LDADD) $(GDAL_LDADD)

