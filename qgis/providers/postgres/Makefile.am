plugindir = ${pkglibdir}
plugin_LTLIBRARIES     = postgresprovider.la
                      
postgresprovider_la_SOURCES = qgspostgresprovider.cpp \
														qgspostgresprovider.h \
														../../src/qgsfeature.cpp \
														../../src/qgsfeatureattribute.cpp \
														../../src/qgsrect.cpp \
														../../src/qgspoint.cpp \
														../../src/qgsfield.cpp       
                       
AM_CXXFLAGS := $(CXXFLAGS) $(EXTRA_CXXFLAGS) $(QT_CXXFLAGS) $(PGDB) -I$(PG_INC) $(DEBUG_QGIS)

postgresprovider_la_LIBADD = $(QT_LDADD) $(PG_LIB)
postgresprovider_la_LDFLAGS = -avoid-version -module
