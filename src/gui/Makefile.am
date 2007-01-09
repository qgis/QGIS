# Copyright (C) 2003 Gary Sherman <sherman at mrcc.com>
#  
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without 
# modifications, as long as this notice is preserved.
# 
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

##
## ----------------------------------------------------------------------
##        Initial setup 
## ----------------------------------------------------------------------
##

%.moc.cpp: %.h
	$(MOC) -o $@ $<

##
## ----------------------------------------------------------------------
##      Define Targets
## ----------------------------------------------------------------------
##
lib_LTLIBRARIES = libqgis_gui.la


##
## ----------------------------------------------------------------------
##         libqgis_gui-gui stuff starts here
## ----------------------------------------------------------------------
##

libqgis_guiHEADERS =							\
		qgisgui.h					\
		qgisinterface.h \
		qgscursors.h			\
		qgsmapcanvas.h						\
		qgsmapcanvasitem.h			\
		qgsmapcanvasmap.h			\
		qgsmapoverviewcanvas.h				\
		qgsmaptool.h						\
		qgsmaptoolpan.h					\
		qgsmaptoolzoom.h					\
		qgsmessageviewer.h					\
		qgsprojectionselector.h		\
		qgsrubberband.h					\
		qgsvertexmarker.h
		

## files generated from MOC
libqgis_gui_la_MOC =							\
		qgisinterface.moc.cpp	\
		qgsmapcanvas.moc.cpp				\
		qgsmapcanvasitem.moc.cpp			\
		qgsmapcanvasmap.moc.cpp			\
		qgsmapoverviewcanvas.moc.cpp		\
		qgsprojectionselector.moc.cpp
		
libqgis_gui_la_SOURCES =						\
		qgisgui.cpp				\
		qgisinterface.cpp	\
		qgscursors.cpp		\
		qgsmapcanvas.cpp					\
		qgsmapcanvasitem.cpp				\
		qgsmapcanvasmap.cpp				\
		qgsmapoverviewcanvas.cpp			\
		qgsmaptool.cpp			\
		qgsmaptoolpan.cpp				\
		qgsmaptoolzoom.cpp			\
		qgsmessageviewer.cpp				\
		qgsprojectionselector.cpp		\
		qgsrubberband.cpp					\
		qgsvertexmarker.cpp
		
AM_YFLAGS       = -d

nodist_libqgis_gui_la_SOURCES = $(libqgis_gui_la_MOC)

BUILT_SOURCES = $(libqgis_gui_la_MOC)


libqgis_gui_la_LIBADD = ../core/libqgis_core.la $(QT_LDADD) $(GEOS_LDADD) $(GDAL_LDADD) $(PYTHON_LIB) -lsqlite3
libqgis_gui_la_LDFLAGS = -version-info $(INTERFACE_VERSION)

libqgis_gui_la_CXXFLAGS = $(PREFIX) $(PLUGINPATH) $(PKGDATAPATH) $(GDAL_CFLAGS) $(CXXFLAGS) $(EXTRA_CXXFLAGS) $(QT_CXXFLAGS) $(PG_INC) $(DEBUG_QGIS) $(GEOS_CFLAGS) $(PYTHON_INCLUDE_DIR) $(HAVE_PYTHON) -I../ui -I../core/include


##
## ----------------------------------------------------------------------
##         additional files that need to be installed go here
## ----------------------------------------------------------------------
##
pkginclude_HEADERS = $(libqgis_guiHEADERS) 

pkginclude_dir = $(includedir)/qgis


CLEANFILES = $(BUILT_SOURCES)

EXTRA_DIST = $(libqgis_gui_la_MOC)

