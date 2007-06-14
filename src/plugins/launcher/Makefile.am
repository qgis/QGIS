# Copyright (C) 2003 Gary Sherman <sherman at mrcc.com> 
#           and 2006 Tim Sutton <tim@linfiniti.com>
# 
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without 
# modifications, as long as this notice is preserved.
# 
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
#
# $Id: Makefile.am 5198 2006-04-06 14:35:09Z timlinux $

##
## Legacy support for pkgdatapath - you are encouraged to use
## Qt4.x resource files where possible rather!
##
if !HAVE_QTMAC
PKGDATAPATH=-DPKGDATAPATH=\"$(pkgdatadir)\"
endif

plugindir = ${pkglibdir}

#for plugin so
plugin_LTLIBRARIES = launcherplugin.la

## For Qt4 intermerdiary meta object compiler files
%.moc.cpp: %.h
	$(MOC) -o $@ $<

## For Qt4 User Interface Files
ui_%.h: %.ui
	$(UIC) -o $@ $<

## For Qt4 Resource Files
%.qrc.cpp: %.qrc
	$(RCC) -o $@ $<

##
##  For plugin lib
##

launcherplugin_la_SOURCES = qgslauncherplugin.cpp \
	qgslauncherplugin.h \
	qgslauncherplugingui.cpp \
	qgslauncherplugingui.h \
	$(plugin_UI)\
	$(plugin_MOC)\
	$(plugin_QRC) \
	$(plugin_RES)


plugin_MOC = qgslauncherplugin.moc.cpp \
    qgslauncherplugingui.moc.cpp 

plugin_UI = ui_qgslauncherpluginguibase.h

plugin_UIC = qgslauncherpluginguibase.ui

#plugin_RES = launcher.qrc.cpp

#plugin_QRC = launcher.qrc

BUILT_SOURCES = $(plugin_MOC) $(plugin_UI) $(plugin_RES)

launcherplugin_la_LIBADD = $(QT_LDADD) \
				$(GDAL_LDADD) \
				../../core/libqgis_core.la \
				../../gui/libqgis_gui.la

launcherplugin_la_CFLAGS = $(CFLAGS) $(EXTRA_CFLAGS) $(DEBUG_QGIS)
launcherplugin_la_CXXFLAGS = $(CXXFLAGS) \
				$(EXTRA_CXXFLAGS) \
				$(QT_CXXFLAGS) \
				$(DEBUG_QGIS) \
				$(GDAL_CFLAGS) \
				$(GEOS_CFLAGS) \
				-I../../core \
				-I../../ui \
				-I../../gui -I../../raster \
				-I..

launcherplugin_la_LDFLAGS = -avoid-version -module
