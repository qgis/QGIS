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
if SPIT
if POSTGRESDB
lib_LTLIBRARIES = libspitplugin.la
%.moc.cpp: %.h
	$(MOC) -o $@ $<

%.h: %.ui
	$(UIC) -o $@ $<

%.cpp: %.ui
	$(UIC) -o $@ -impl $*.h $<
	
libspitplugin_la_SOURCES =  qgsspit.cpp \
        qgsspitplugin.cpp \
				qgsspitplugin.h \
				qgsconnectiondialog.cpp \
				qgsshapefile.cpp \
				qgsmessageviewer.cpp \
				$(spit_UI)\
				$(spit_MOC)
        
spit_MOC = 	qgsspit.moc.cpp \
        qgsspitplugin.moc.cpp \
				qgsconnectiondialog.moc.cpp \
				qgsshapefile.moc.cpp \
				qgsmessageviewer.moc.cpp \
				qgsconnectiondialogbase.moc.cpp \
				qgsspitbase.moc.cpp \
				qgsmessageviewerbase.moc.cpp

spit_UI = qgsconnectiondialogbase.h \
	qgsconnectiondialogbase.cpp \
	qgsspitbase.h \
	qgsspitbase.cpp \
	qgsmessageviewerbase.h \
	qgsmessageviewerbase.cpp
        
spit_UIC = qgsconnectiondialogbase.ui \
	qgsspitbase.ui \
	qgsmessageviewerbase.ui

# UI dependencies			
qgsconnectiondialogbase.cpp: qgsconnectiondialogbase.ui qgsconnectiondialogbase.h
qgsconnectiondialogbase.h: qgsconnectiondialogbase.ui
qgsspitbase.cpp: qgsspitbase.h qgsspitbase.ui
qgsspitbase.h: qgsspitbase.ui
qgsmessageviewerbase.cpp: qgsmessageviewerbase.h qgsmessageviewerbase.ui
qgsmessageviewerbase.h: qgsmessageviewerbase.ui

BUILT_SOURCES = $(spit_MOC) $(spit_UI)

AM_CXXFLAGS := $(CXXFLAGS) $(EXTRA_CXXFLAGS) $(GDAL_CFLAGS) $(QT_CXXFLAGS) $(PGDB) -I$(PG_INC) 

libspitplugin_la_LIBADD = $(QT_LDADD)  $(PG_LIB) $(GDAL_LIB)
endif
endif
