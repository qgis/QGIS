# Copyright (C) 2003 Gary Sherman <sherman at mrcc.com>
#  
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without 
# modifications, as long as this notice is preserved.
# 
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
lib_LTLIBRARIES = exampleplugin.la
%.moc.cpp: %.h
	$(MOC) -o $@ $<

%.h: %.ui
	$(UIC) -o $@ $<

%.cpp: %.ui
	$(UIC) -o $@ -impl $*.h $<
	
exampleplugin_la_SOURCES =  exampleplugin.cpp \
				$(example_UI)\
				$(example_MOC)
        
example_MOC = 	exampleplugin.moc.cpp

example_UI = 

# UI dependencies			

AM_CXXFLAGS := $(CXXFLAGS) $(EXTRA_CXXFLAGS) $(QT_CXXFLAGS) 

exampleplugin_la_LIBADD = $(QT_LDADD) 
exampleplugin_la_LDFLAGS = -avoid-version -module
