# Copyright (C) 2003 Gary Sherman <sherman at mrcc.com>
#  
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without 
# modifications, as long as this notice is preserved.
# 
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
bin_PROGRAMS = spit
%.moc.cpp: %.h
	$(MOC) -o $@ $<

%.h: %.ui
	$(UIC) -o $@ $<

%.cpp: %.ui
	$(UIC) -o $@ -impl $*.h $<
	
spit_SOURCES =  main.cpp \
  qgsspit.cpp \
	qgsconnectiondialog.cpp \
	qgsshapefile.cpp \
	qgsmessageviewer.cpp \
				$(spit_UI)\
				$(spit_MOC)
spit_MOC = 	qgsconnectiondialogbase.moc.cpp \
  qgsshapefile.moc.cpp \
  qgsmessageviewerbase.moc.cpp \
  qgsspitbase.moc.cpp

spit_UI = qgsconnectiondialogbase.h \
    qgsconnectiondialogbase.cpp \
    qgsspitbase.h \
    qgsspitbase.cpp \
    qgsmessageviewerbase.h \
    qgsmessageviewerbase.cpp

# UI dependencies			
qgsconnectiondialogbase.h: qgsconnectiondialogbase.ui
qgsconnectiondialogbase.cpp: qgsconnectiondialogbase.ui qgsconnectiondialogbase.h
qgsspitbase.h: qgsspitbase.ui
qgsspitbase.cpp: qgsspitbase.h qgsspitbase.ui
qgsmessageviewerbase.h: qgsmessageviewerbase.ui
qgsmessageviewerbase.cpp: qgsmessageviewerbase.h qgsmessageviewerbase.ui

# add required defines for compiling PostgreSQL
PGDB = -DPOSTGRESQL -DHAVE_NAMESPACE_STD -DHAVE_CXX_STRING_HEADER -DDLLIMPORT=""

BUILT_SOURCES = $(spit_MOC) $(spit_UI)
spit_LDADD := $(LDADD) $(QT_LDADD) $(GDAL_LIB) $(PG_LIB)
AM_CXXFLAGS := $(PGDB) $(CXXFLAGS) $(EXTRA_CXXFLAGS) -I../include $(QT_CXXFLAGS) -I$(PG_INC) -g 
CLEANFILES = $(BUILT_SOURCES)
