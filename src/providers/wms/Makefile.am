INCLUDES = -I../../core/include

%.moc.cpp: %.h
	$(MOC) -o $@ $<

plugindir = ${pkglibdir}
plugin_LTLIBRARIES     = wmsprovider.la


## non-uic files generated from MOC

wmsprovider_MOC =	qgswmsprovider.moc.cpp			\
		../../core/qgsdataprovider.moc.cpp		\
		../../core/qgsrasterdataprovider.moc.cpp		
                
wmsprovider_la_SOURCES =					\
		qgswmsprovider.cpp				\
		qgswmsprovider.moc.cpp				\
		qgswmsprovider.h				

BUILT_SOURCES = $(wmsprovider_MOC)
                                
wmsprovider_la_LIBADD = $(QT_LDADD) ../../core/libqgis_core.la
wmsprovider_la_LDFLAGS = -avoid-version -module
wmsprovider_la_CXXFLAGS = $(EXTRA_CXXFLAGS) $(QT_CXXFLAGS) $(DEBUG_QGIS) $(GDAL_CFLAGS)

CLEANFILES = $(BUILT_SOURCES)
