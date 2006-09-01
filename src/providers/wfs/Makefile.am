INCLUDES = -I../../core/ -I../../gui/ -I../../ui

%.moc.cpp: %.h
	$(MOC) -o $@ $<

wfsprovider_MOC = qgshttptransaction.moc.cpp

plugindir = ${pkglibdir}
plugin_LTLIBRARIES = wfsprovider.la

wfsprovider_la_SOURCES = qgswfsprovider.cpp

BUILT_SOURCES = $(wfsprovider_MOC)

wfsprovider_la_LIBADD = $(QT_LDADD)  $(GDAL_LIB) $(GEOS_LDADD) ../../gui/libqgis_gui.la ../../core/libqgis_core.la
wfsprovider_la_LDFLAGS = -avoid-version -module
wfsprovider_la_CXXFLAGS = $(GDAL_CFLAGS) $(EXTRA_CXXFLAGS) $(QT_CXXFLAGS) $(DEBUG_QGIS) $(GEOS_CFLAGS)

CLEANFILES = $(BUILT_SOURCES)