# Copyright (C) 2005 Tom Elwertowski
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

# This is the Contents directory of a Mac OS bundle
# (configure ensures that prefix ends in qgis.app/Contents/MacOS)

contentsdir = $(exec_prefix)/../

contents_DATA = \
	Info.plist \
	Pkginfo

Info.plist: Info.plist.in $(top_builddir)/configure.in
	@SHORTVERSION=$(MAJOR_VERSION).$(MINOR_VERSION).$(MICRO_VERSION) && \
	sed -e "s/@VERSION/$(VERSION)/" \
		-e "s/@SHORTVERSION/$$SHORTVERSION/" \
		< Info.plist.in > $@

Pkginfo: Makefile
	@echo -n "APPLQGIS" > $@

# Update bundle modification date so Finder scans latest plist
install-exec-hook:
	touch $(contentsdir)/../.

EXTRA_DIST = \
	Info.plist.in

SUBDIRS = Resources
