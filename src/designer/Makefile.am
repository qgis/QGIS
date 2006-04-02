# Copyright (C) 2005 Tim Sutton <tim at linfiniti.com>
#  
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without 
# modifications, as long as this notice is preserved.
# 
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
#
# $Id: Makefile.am 4490 2006-01-07 01:09:27Z timlinux $

##
## ----------------------------------------------------------------------
##      Define Targets
## ----------------------------------------------------------------------
##
lib_LTLIBRARIES= libqgis_designerwidgets.la
## TODO change this so it installs into ${QTDIR}/plugins/designer
libqgis_designerwidgetsdir = ${pkglibdir}/designer

##
## ----------------------------------------------------------------------
##        get Qt4 to run moc and rcc where required... 
## ----------------------------------------------------------------------
##

#run moc on files
%.moc.cpp: %.h
	$(MOC) -o $@ $<

#qt resources
%.qrc.cpp: %.qrc
	$(RCC) -o $@ $<

libqgis_designerwidgets_la_MOC = qgsprojectionselectorwidget.moc.cpp

libqgis_designerwidgets_RES = qgisdesignerwidgets.qrc.cpp

libqgis_designerwidgets_QRC = qgisdesignerwidgets.qrc


##		$(libqgis_designerwidgets_la_MOC) \
libqgis_designerwidgets_la_SOURCES =  qgsprojectionselectorwidget.cpp \
		$(libqgis_designerwidgets_QRC) \
		$(libqgis_designerwidgets_RES)


libqgis_designerwidgets_la_LIBADD =  $(QT_LDADD) ../widgets/projectionselector/libqgsprojectionselector.la
libqgis_designerwidgets_la_LDFLAGS = -avoid-version -module
libqgis_designerwidgets_la_CXXFLAGS = $(CXXFLAGS) $(EXTRA_CXXFLAGS) $(QT_CXXFLAGS) $(DEBUG_QGIS) -I../gui/ -I../legend -I../core -I../widgets/projectionselector/

##
## ----------------------------------------------------------------------
##         additional files that need to be installed go here
## ----------------------------------------------------------------------
##
libqgis_designerwidgets_HEADERS =  qgsprojectionselectorwidget.h 
