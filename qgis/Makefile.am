# Copyright (C) 2003 Gary Sherman <sherman at mrcc.com>
#  
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without 
# modifications, as long as this notice is preserved.
# 
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

#EXTRA_DIST = doc
SUBDIRS = src providers plugins

install-data-local:
	for f in `(cd $(srcdir)/doc; find -type d)`; do \
	$(INSTALL) -d $(datadir)/doc/$$f; \
	done; \
	for f in `(cd $(srcdir)/doc; find -type f)`; do \
	$(INSTALL_DATA) $(srcdir)/doc/$$f $(datadir)/doc/$$f; \
	done;

