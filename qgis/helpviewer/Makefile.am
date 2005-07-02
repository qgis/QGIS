# Copyright (C) 2005 Gary Sherman gsherman at mrcc dot com
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


bin_PROGRAMS = qgis_help

PKGDATAPATH=-DPKGDATAPATH=\"$(pkgdatadir)\"


%.moc.cpp: %.h
	$(MOC) -o $@ $<

%.h: %.ui
	$(UIC) -o $@ $<

%.cpp: %.ui
	$(UIC) -o $@ -impl $*.h $<



qgis_help_MOC = qgshelpviewer.moc.cpp \
								qgshelpviewerbase.moc.cpp

qgis_help_UI = qgshelpviewerbase.h \
							 qgshelpviewerbase.cpp

qgis_help_UIC = qgshelpviewerbase.ui

qgis_help_SOURCES = main.cpp \
									qgshelpviewer.h \
									qgshelpviewer.cpp \
				       $(qgis_help_UI)			\
				       $(qgis_help_MOC)

BUILT_SOURCES = $(qgis_help_MOC) $(qgis_help_UI)

qgis_help_LDADD = $(LDADD) $(QT_LDADD) -lsqlite3

qgis_help_CXXFLAGS = $(CXXFLAGS) $(EXTRA_CXXFLAGS) $(QT_CXXFLAGS) $(DEBUG_QGIS) $(PKGDATAPATH)

#EXTRA_DIST = $(qgis_help_UIC) icon.xpm

