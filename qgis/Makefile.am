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
	doc/skin/page.css \
	doc/skin/breadcrumbs-optimized.js \
	doc/skin/krysalis.css \
	doc/skin/images/tab-top.png \
	doc/skin/images/favicon.ico \
	doc/skin/images/icon_warning_sml.gif \
	doc/skin/images/icon_warning_lrg.gif \
	doc/skin/images/icon_doc_sml.gif \
	doc/skin/images/icon_doc_lrg.gif \
	doc/skin/images/xmldoc.gif \
	doc/skin/images/chapter.gif \
	doc/skin/images/singlepage.gif \
	doc/skin/images/page.gif \
	doc/skin/images/label.gif \
	doc/skin/images/vcss.png \
	doc/skin/images/icon_folder_sml.gif \
	doc/skin/images/nav-bottom.png \
	doc/skin/images/icon_folder_lrg.gif \
	doc/skin/images/search-bottom.png \
	doc/skin/images/icon_help_sml.gif \
	doc/skin/images/current.gif \
	doc/skin/images/navbullet-selected.png \
	doc/skin/images/bottom-left.png \
	doc/skin/images/printer.gif \
	doc/skin/images/printer.png \
	doc/skin/images/icon_success_sml.gif \
	doc/skin/images/pdfdoc.gif \
	doc/skin/images/icon_success_lrg.gif \
	doc/skin/images/icon_error_sml.gif \
	doc/skin/images/valid-html401.png \
	doc/skin/images/icon_error_lrg.gif \
	doc/skin/images/content-bg.gif \
	doc/skin/images/top-right.png \
	doc/skin/images/bottom-right.png \
	doc/skin/images/top-left.png \
	doc/skin/images/navbullet.png \
	doc/skin/images/spacer.gif \
	doc/skin/images/icon_info_sml.gif \
	doc/skin/images/icon_info_lrg.gif \
	doc/skin/images/chapter_open.gif \
	doc/skin/breadcrumbs.js \
	doc/userguide.html \
	doc/favicon.ico \
	doc/install.pdf \ 
	doc/README \
	doc/index.html \
	doc/install.html \
	doc/images/gis_add_raster_layer.png \
	doc/images/change_symbology.gif \
	doc/images/qgis_raster_properties_symbology_tab.png \
	doc/images/qgis_mainwindow01.png \
	doc/images/project.png \
	doc/images/loaded_shapefiles.gif \
	doc/images/add_layer_dialog.gif \
	doc/images/qgis_raster_popup_properties_selected.png \
	doc/images/built-with-forrest-button.png \
	doc/images/layer_properties.gif \
	doc/images/qgis_raster_properties_general_tab.png \
	doc/images/qgis_raster_properties_statistics_tab.png \
	doc/images/qgis_load_data_on_startup.png \
	doc/images/btn_add_layer.gif \
	doc/images/changed_symbology.gif \
	doc/userguide.pdf \
	doc/index.pdf \
	doc/plugins/geoprocessing/buffer/bufferdialog.png \
	doc/plugins/geoprocessing/buffer/pluginmanager.png \
	doc/plugins/geoprocessing/buffer/index.html \
	doc/plugins/geoprocessing/buffer/bufferresult.png \
	doc/plugins/geoprocessing/buffer/alaska_pglayer.png \
	doc/plugins/geoprocessing/buffer/buffer.png \
	doc/plugins/geoprocessing/buffer/buffericon.png

SUBDIRS = src providers plugins

install-data-local:
	chmod 755 `find $(srcdir)/doc -type d` $(srcdir)/doc
	chmod 644 `find $(srcdir)/doc -type f`
	rm -rf `find $(srcdir)/doc -name CVS`
	for f in `(cd $(srcdir)/doc; find -type d)`; do \
	$(INSTALL) -d $(datadir)/doc/$$f; \
	done; \
	for f in `(cd $(srcdir)/doc; find -type f)`; do \
	$(INSTALL_DATA) $(srcdir)/doc/$$f $(datadir)/doc/$$f; \
	done;

release: distdir
	-chmod -R a+r $(distdir)
	GZIP=$(GZIP_ENV) $(AMTAR)$(TAR) chozf $(distdir).tar.gz $(distdir)
	BZIP2=$(BZIP2_ENV) $(AMTAR)$(TAR) --bzip2 -chof $(distdir).tar.bz2 $(distdir)
	-rm -rf $(distdir)

#rm -f $(distdir)/sylpheed.spec
#sed '/^Source:/s/\.tar\.gz$$/\.tar\.bz2/' sylpheed.spec \
#		> $(distdir)/sylpheed.spec
