INCLUDES = -I../../src

plugindir = ${pkglibdir}
plugin_LTLIBRARIES     = postgresprovider.la


postgresprovider_la_SOURCES = qgspostgresprovider.cpp 		\
			      qgspostgresprovider.h		\
			      ../../src/qgsfeature.cpp		\
			      ../../src/qgsfeatureattribute.cpp	\
			      ../../src/qgsrect.cpp		\
			      ../../src/qgspoint.cpp		\
			      ../../src/qgsfield.cpp       

postgresprovider_la_LIBADD = $(QT_LDADD) $(PG_LIB) $(GEOS_LDADD)
postgresprovider_la_LDFLAGS = -avoid-version -module
postgresprovider_la_CXXFLAGS = $(EXTRA_CXXFLAGS) $(QT_CXXFLAGS) -I$(PG_INC) $(DEBUG_QGIS) $(GEOS_CFLAGS)
