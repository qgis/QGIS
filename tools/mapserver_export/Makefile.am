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
if HAVE_QTMAC
MAC = mac
endif

if !HAVE_QTMAC
PREFIX=-DPREFIX=\"$(prefix)\"
PLUGINPATH=-DPLUGINPATH=\"$(pkglibdir)\"
PKGDATAPATH=-DPKGDATAPATH=\"$(pkgdatadir)\"
endif

%.moc.cpp: %.h
	$(MOC) -o $@ $<

ui_%.h: %.ui
	$(UIC) -o $@ $<

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
							ms_main.cpp \
							msexport_wrap.cxx \
	$(msexport_UI) \
	$(msexport_UIC) \
	$(msexport_MOC)



# UI dependencies			
qgsmapserverexportbase.cpp: qgsmapserverexportbase.ui qgsmapserverexportbase.h
qgsmapserverexportbase.h: qgsmapserverexportbase.ui

BUILT_SOURCES = $(msexport_MOC) $(msexport_UI) 



msexport_LDADD = $(QT_LDADD) $(PG_LIB) $(PYTHON_LIB) $(PROJ_LIB) ../../src/core/libqgis_core.la ../../src/gui/libqgis_gui.la 

msexport_CXXFLAGS = $(CXXFLAGS) $(PKGDATAPATH) $(EXTRA_CXXFLAGS) $(QT_CXXFLAGS) $(PYTHON_INCLUDE_DIR) -I../../src

CLEANFILES = $(BUILT_SOURCES)

lib_LTLIBRARIES = libmsexport.la

libmsexport_la_SOURCES = qgsmapserverexport.cpp qgsmapserverexport.moc.cpp msexport_wrap.cxx

libmsexport_la_LIBADD = $(PYTHON_LIB) $(QT_LDADD)
libmsexport_la_LDFLAGS = -version-info $(INTERFACE_VERSION)
libmsexport_la_CXXFLAGS = $(CXXFLAGS) $(PKGDATAPATH) $(EXTRA_CXXFLAGS) $(QT_CXXFLAGS) $(PYTHON_INCLUDE_DIR) -I../../src

# Qt/Mac app needs to be inside a bundle to function correctly
MACBINDIR = $(bindir)/msexport.app/Contents/MacOS

install-exec-hook:
if HAVE_QTMAC
	$(mkinstalldirs) $(MACBINDIR)
	mv $(bindir)/msexport $(MACBINDIR)
endif


#
# Put script into an appropriate dir
#
pythondir = ${pkgdatadir}/python
if HAVE_QTMAC
	pythondir=$(MACBINDIR)/share/qgis/python
endif

python_DATA = *.py

EXTRA_DIST = $(msexport_UIC) \
             $(python_DATA)
