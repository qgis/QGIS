EXTRA_DIST = example/Makefile.am \
							example/Makefile.in \
							example/exampleplugin.cpp \
							example/exampleplugin.h \
							example/matrix1.xpm \
							maplayer/Makefile.am \
							maplayer/Makefile.in \
							maplayer/maplayertest.cpp \
							maplayer/maplayertest.h \
							maplayer/matrix1.xpm \
							qgisplugin.h

if HAVE_SPIT
SPIT = spit
endif

if HAVE_POSTGRESDB
GEOPROCESSING = geoprocessing
endif

SUBDIRS = $(GEOPROCESSING) $(SPIT)
