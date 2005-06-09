INCLUDES = -I../../src

%.moc.cpp: %.h
	$(MOC) -o $@ $<

plugindir = ${pkglibdir}
plugin_LTLIBRARIES     = postgresprovider.la


## non-uic files generated from MOC

postgresprovider_MOC =	qgspostgresprovider.moc.cpp

                
postgresprovider_la_SOURCES =					\
		qgspostgresprovider.cpp				\
		qgspostgresprovider.moc.cpp			\
		qgspostgresprovider.h				\
		../../src/qgsfeature.cpp			\
		../../src/qgsfeatureattribute.cpp		\
		../../src/qgsrect.cpp				\
		../../src/qgspoint.cpp				\
		../../src/qgsfield.cpp				\
		qgspostgisbox2d.cpp				\
		qgspostgisbox2d.h				\
		qgspostgisbox3d.cpp				\
		qgspostgisbox3d.h				\
		qgspostgrescountthread.cpp			\
		qgspostgrescountthread.h			\
		qgspostgresextentthread.cpp			\
		qgspostgresextentthread.h

BUILT_SOURCES = $(postgresprovider_MOC)
                                
postgresprovider_la_LIBADD = $(QT_LDADD) $(PG_LIB) $(GEOS_LDADD)
postgresprovider_la_LDFLAGS = -avoid-version -module
postgresprovider_la_CXXFLAGS = $(EXTRA_CXXFLAGS) $(QT_CXXFLAGS) -I$(PG_INC) $(DEBUG_QGIS) $(GEOS_CFLAGS) $(GDAL_CFLAGS)

CLEANFILES = $(BUILT_SOURCES)
