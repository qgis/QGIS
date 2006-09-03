INCLUDES = -I../../core/ -I../../gui/ -I../../ui

%.moc.cpp: %.h
	$(MOC) -o $@ $<

libwfsprovider_MOC = qgshttptransaction.moc.cpp

plugindir = ${pkglibdir}
plugin_LTLIBRARIES = libwfsprovider.la

libwfsprovider_la_SOURCES = qgswfsprovider.cpp qgshttptransaction.cpp $(libwfsprovider_MOC)

BUILT_SOURCES = $(wfsprovider_MOC)

libwfsprovider_la_LIBADD = $(QT_LDADD)  $(GDAL_LIB) $(GEOS_LDADD) ../../gui/libqgis_gui.la ../../core/libqgis_core.la
libwfsprovider_la_LDFLAGS = -avoid-version -module
libwfsprovider_la_CXXFLAGS = $(GDAL_CFLAGS) $(EXTRA_CXXFLAGS) $(QT_CXXFLAGS) $(DEBUG_QGIS) $(GEOS_CFLAGS)

CLEANFILES = $(BUILT_SOURCES)