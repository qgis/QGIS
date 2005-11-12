# Copyright (C) 2004 Gary Sherman <sherman at mrcc.com>
#  
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without 
# modifications, as long as this notice is preserved.
# 
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
#
# $Id: Makefile.am 4200 2005-11-12 01:37:49Z gsherman $

bin_PROGRAMS = msexport


%.moc.cpp: %.h
	$(MOC) -o $@ $<

%.h: %.ui 
	$(UIC) -o $@ $<
if HAVE_QT4
	../../tools/qgis-qt3to4.pl -uic $@
endif

if !NO_UIC_IMPLEMENTATIONS
%.cpp: %.ui
	$(UIC) -o $@ -impl $*.h $<
endif

# %.h: %.ui
# 	$(UIC) -o $@ $<
#
# %.cpp: %.ui
# 	$(UIC) -o $@ -impl $*.h $<

msexport_MOC = qgsmapserverexport.moc.cpp \
							 qgsmapserverexportbase.moc.cpp


msexport_UI = qgsmapserverexportbase.cpp \
	qgsmapserverexportbase.h

msexport_UIC = qgsmapserverexportbase.ui 

msexport_SOURCES =  qgsmapserverexport.cpp \
										qgsmapserverexport.h \
							main.cpp \
	$(msexport_UI) \
	$(msexport_UIC) \
	$(msexport_MOC)



# UI dependencies			
qgsmapserverexportbase.cpp: qgsmapserverexportbase.ui qgsmapserverexportbase.h
qgsmapserverexportbase.h: qgsmapserverexportbase.ui

BUILT_SOURCES = $(msexport_MOC) $(msexport_UI) 



msexport_LDADD = $(QT_LDADD) $(PYTHON_LIB)
msexport_CXXFLAGS = $(CXXFLAGS) $(EXTRA_CXXFLAGS) $(QT_CXXFLAGS) $(PYTHON_INCLUDE_DIRS) -I../../src

CLEANFILES = $(BUILT_SOURCES)

EXTRA_DIST = $(msexport_UIC) 
