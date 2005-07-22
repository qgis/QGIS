# Copyright (C) 2005 Jens Oberender <j.obi at troja.net>
#
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without
# modifications, as long as this notice is preserved.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
#
# $Id$

if !HAVE_QTMAC
PKGDATAPATH=-DPKGDATAPATH=\"$(pkgdatadir)\"
endif

noinst_LTLIBRARIES = libqgis_legend.la

%.moc.cpp: %.h
	$(MOC) -o $@ $<

qgis_MOC = qgslegend.moc.cpp

libqgis_legend_la_SOURCES =	qgslegendgroup.cpp \
				qgslegend.cpp \
				qgslegend.moc.cpp \
				qgslegenditem.cpp \
				qgslegendlayer.cpp \
				qgslegendlayerfile.cpp \
				qgslegendlayerfilegroup.cpp \
				qgslegendpropertygroup.cpp \
				qgslegendpropertyitem.cpp \
				qgslegendsymbologygroup.cpp \
				qgslegendsymbologyitem.cpp \
				qgslegendvectorsymbologyitem.cpp

libqgis_legend_la_LIBFLAGS = $(QT_LDADD)

libqgis_legend_la_LDFLAGS = -version-info $(INTERFACE_VERSION)
libqgis_legend_la_CXXFLAGS = $(PKGDATAPATH) -I.. \
				$(CXXFLAGS) $(EXTRA_CXXFLAGS) \
				$(QT_CXXFLAGS) $(DEBUG_QGIS)
