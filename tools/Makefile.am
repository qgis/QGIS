# Copyright (C) 2004 Jens Oberender <j.obi at troja.net>
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

SUBDIRS = qgis_config

m4filedir = $(datadir)/aclocal

m4file_DATA = qgis.m4

m4file_SOURCE = qgisdetect.m4

$(m4file_DATA): $(m4file_SOURCE)
	cat ../acinclude.m4 $(m4file_SOURCE) > $(m4file_DATA)

EXTRA_DIST = $(m4file_SOURCE)

clean:
	rm -f $(m4file_DATA)
