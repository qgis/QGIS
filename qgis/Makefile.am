# Copyright (C) 2003 Gary Sherman <sherman at mrcc.com>
#  
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without 
# modifications, as long as this notice is preserved.
# 
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

EXTRA_DIST = \
	AUTHORS \
	BUGS \
	ChangeLog \
	COPYING \
	create_qm_files.sh \
	doc \
	INSTALL \
	$(man1_MANS) \
	NEWS \
	qgis.spec \
	qgsconfig.h.in \
	README \
	TODO \
	update_ts_files.sh

man1_MANS = qgis.man

SUBDIRS = src providers plugins doc tools i18n

pkginclude_HEADERS = qgsconfig.h

UIcheck:
	@UIS=`find . -name *.ui` && \
	if [ `grep "UI version" $$UIS | sed -e '/3.1/d' | wc -l` -ne 0 ]; then \
		echo -e "\nWrong UI version:"; \
		grep "UI version" $$UIS | sed -e '/3.1/d' -e 's/:.*version="\([0-9\.]*\)".*/ is version \1/g'; \
		echo; \
		exit 1; \
	fi

release: UIcheck distdir
	-chmod -R a+r $(distdir)
	GZIP=$(GZIP_ENV) $(AMTAR)$(TAR) chozf $(distdir).tar.gz $(distdir)
	rm -f $(distdir)/qgis.spec
	sed '/^Source:/s/\.tar\.gz$$/\.tar\.bz2/' qgis.spec \
			> $(distdir)/qgis.spec
	BZIP2=$(BZIP2_ENV) $(AMTAR)$(TAR) --bzip2 -chof $(distdir).tar.bz2 $(distdir)
	-rm -rf $(distdir)
