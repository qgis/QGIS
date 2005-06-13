# Copyright (C) 2003 Gary Sherman <sherman at mrcc.com>
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


libdir    = ${prefix}/lib/qgis
pkglibdir = ${libdir}

%.moc.cpp: %.h
	$(MOC) -o $@ $<

%.moc.uic.cpp: %.uic.h
	$(MOC) -o $@ $<

%.uic.h: %.ui 
	$(UIC) -o $@ $<

%.uic.cpp: %.ui
	$(UIC) -o $@ -impl $*.uic.h $<

## target plug-in 
lib_LTLIBRARIES = climatedataprocessingwizard.la

## target binary file
bin_PROGRAMS = cdpwizard

## qt designer user-interface files
plugin_UI = cdpwizardbase.ui

## uic created headers
plugin_UIHEADERS = $(plugin_UI:.ui=.uic.h)

## uic created source files
plugin_UISOURCES = $(plugin_UI:.ui=.uic.cpp)

## uic created MOC files
plugin_UIMOC = $(plugin_UI:.ui=.moc.uic.cpp)

## MOC files
plugin_MOC = plugin.moc.cpp cdpwizard.moc.cpp climatedataprocessor.moc.cpp


climatedataprocessingwizard_la_SOURCES = plugin.cpp \
	plugin.h \
	cdpwizard.cpp \
	cdpwizard.h \
	climatedataprocessor.cpp \
	climatedataprocessor.h \
	dataprocessor.cpp \
	dataprocessor.h \
	filegroup.cpp \
	filegroup.h \
	filereader.cpp \
	filereader.h \
	filewriter.cpp \
	filewriter.h \
	testdatamaker.cpp \
	testdatamaker.h \
	meridianswitcher.h \
	meridianswitcher.cpp \
	imagewriter.h \
	imagewriter.cpp \
	$(plugin_UISOURCES) \
	$(plugin_UIHEADERS) \
	$(plugin_UIMOC)
	
nodist_climatedataprocessingwizard_la_SOURCES =	$(plugin_MOC)

climatedataprocessingwizard_la_CXXFLAGS = $(CXXFLAGS) $(EXTRA_CXXFLAGS) $(QT_CXXFLAGS) $(QGIS_CXXFLAGS) $(GDAL_CFLAGS) $(GSL_FLAGS) $(DEBUG_QGIS)
climatedataprocessingwizard_la_LDFLAGS = -avoid-version -module $(QGIS_LDADD) $(QT_LDADD) $(GDAL_LDADD) $(GSL_LDADD)

BUILT_SOURCES = $(plugin_UIHEADERS) $(plugin_UISOURCES) $(plugin_UIMOC) $(plugin_MOC)




##
##  For standalone executable
##

cdpwizard_SOURCES = 	main.cpp \
			filereader.cpp \
			filewriter.cpp \
			filegroup.cpp \
			climatedataprocessor.cpp \
			dataprocessor.cpp \
			cdpwizard.cpp \
			testdatamaker.cpp \
			testdatamaker.h \
			meridianswitcher.h \
			meridianswitcher.cpp \
			imagewriter.h \
			imagewriter.cpp \
			$(plugin_UISOURCES) \
			$(plugin_UIHEADERS) \
			$(plugin_UIMOC)

nodist_cdpwizard_SOURCES = cdpwizard.moc.cpp climatedataprocessor.moc.cpp

cdpwizard_LDADD = -lgdal $(QT_LDADD) $(GDAL_LDADD) -lgsl -lgslcblas
cdpwizard_CXXFLAGS = $(CXXFLAGS) $(EXTRA_CXXFLAGS) $(QT_CXXFLAGS) $(DEBUG_QGIS) $(GDAL_CFLAGS) $(GSL_FLAGS)



CLEANFILES = $(BUILT_SOURCES)

EXTRA_DIST = $(plugin_UI) icon.xpm





