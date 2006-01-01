# Copyright (C) 2005 Tim Sutton tim@linfiniti.com
#
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without
# modifications, as long as this notice is preserved.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
#
# $Id: Makefile.am 4416 2005-12-30 17:30:36Z timlinux $

if !HAVE_QTMAC
PKGDATAPATH=-DPKGDATAPATH=\"$(pkgdatadir)\"
endif

lib_LTLIBRARIES = libqgis_composer.la

%.moc.cpp: %.h
	$(MOC) -o $@ $<

libqgis_composer_MOC = qgscomposer.moc.cpp	\
		qgscomposerlabel.moc.cpp			\
		qgscomposerpicture.moc.cpp			\
		qgscomposermap.moc.cpp				\
		qgscomposerscalebar.moc.cpp			\
		qgscomposervectorlegend.moc.cpp		\
		qgscomposerview.moc.cpp				\
		qgscomposition.moc.cpp				

##libqgis_composer_la_HEADERS = qgscomposer.h\
##		qgscomposeritem.h					\
##		qgscomposerlabel.h					\
##		qgscomposerpicture.h				\
##		qgscomposermap.h					\
##		qgscomposerscalebar.h				\
##		qgscomposervectorlegend.h			\
##		qgscomposerview.h					\
##		qgscomposition.h					

libqgis_composer_la_SOURCES =  qgscomposer.cpp\
		qgscomposeritem.cpp					\
		qgscomposerlabel.cpp				\
		qgscomposerpicture.cpp				\
		qgscomposermap.cpp					\
		qgscomposerscalebar.cpp				\
		qgscomposervectorlegend.cpp			\
		qgscomposerview.cpp					\
		qgscomposition.cpp					\
		$(libqgis_composer_MOC)

libqgis_composer_la_LIBFLAGS = $(QT_LDADD)

libqgis_composer_la_LDFLAGS = -version-info $(INTERFACE_VERSION)
libqgis_composer_la_CXXFLAGS = $(PKGDATAPATH) -I.. -I../ui/ \
				$(CXXFLAGS) $(EXTRA_CXXFLAGS) \
				$(GDAL_CFLAGS) $(QT_CXXFLAGS) $(DEBUG_QGIS) $(GEOS_CFLAGS)


BUILT_SOURCES = $(libqgis_composer_MOC)

CLEANFILES = $(BUILT_SOURCES) 
