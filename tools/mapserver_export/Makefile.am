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

ui_%.h: %.ui
	uic -o $@ $<

#uic $< -o $@

# %.h: %.ui
# 	$(UIC) -o $@ $<
#
# %.cpp: %.ui
# 	$(UIC) -o $@ -impl $*.h $<

msexport_MOC = qgsmapserverexport.moc.cpp 


msexport_UI = ui_qgsmapserverexportbase.h

msexport_UIC = qgsmapserverexportbase.ui 

msexport_SOURCES =  qgsmapserverexport.cpp \
										qgsmapserverexport.h \
							main.cpp \
							msexport_wrap.cxx \
	$(msexport_UI) \
	$(msexport_UIC) \
	$(msexport_MOC)



# UI dependencies			
qgsmapserverexportbase.cpp: qgsmapserverexportbase.ui qgsmapserverexportbase.h
qgsmapserverexportbase.h: qgsmapserverexportbase.ui

BUILT_SOURCES = $(msexport_MOC) $(msexport_UI) 



msexport_LDADD = $(QT_LDADD) $(PYTHON_LIB)
msexport_CXXFLAGS = $(CXXFLAGS) $(EXTRA_CXXFLAGS) $(QT_CXXFLAGS) $(PYTHON_INCLUDE_DIR) -I../../src

CLEANFILES = $(BUILT_SOURCES)

EXTRA_DIST = $(msexport_UIC) 

lib_LTLIBRARIES = libmsexport.la

libmsexport_la_SOURCES = qgsmapserverexport.cpp qgsmapserverexport.moc.cpp msexport_wrap.cxx

libmsexport_la_LIBADD = $(PYTHON_LIB) $(QT_LDADD)
libmsexport_la_CXXFLAGS = $(CXXFLAGS) $(EXTRA_CXXFLAGS) $(QT_CXXFLAGS) $(PYTHON_INCLUDE_DIR) -I../../src
#msexport_la_LDFLAGS
