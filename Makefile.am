# Copyright (C) 2003 Gary Sherman <sherman at mrcc.com>
#  
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without 
# modifications, as long as this notice is preserved.
# 
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

EXTRA_DIST = doc \
	AUTHORS \
	ChangeLog \
	README \
	BUGS \
	INSTALL \
	TODO \
	COPYING \
	NEWS \
	qgis.spec \
	config.h.in

SUBDIRS = src providers plugins doc

release: distdir
	-chmod -R a+r $(distdir)
	GZIP=$(GZIP_ENV) $(AMTAR)$(TAR) chozf $(distdir).tar.gz $(distdir)
	rm -f $(distdir)/qgis.spec
	sed '/^Source:/s/\.tar\.gz$$/\.tar\.bz2/' qgis.spec \
			> $(distdir)/qgis.spec
	BZIP2=$(BZIP2_ENV) $(AMTAR)$(TAR) --bzip2 -chof $(distdir).tar.bz2 $(distdir)
	-rm -rf $(distdir)
