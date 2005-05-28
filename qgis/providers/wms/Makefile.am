INCLUDES = -I../../src

%.moc.cpp: %.h
	$(MOC) -o $@ $<

plugindir = ${pkglibdir}
plugin_LTLIBRARIES     = wmsprovider.la


## non-uic files generated from MOC

wmsprovider_MOC =	qgswmsprovider.moc.cpp			\
			qgshttptransaction.moc.cpp
                
wmsprovider_la_SOURCES =					\
		../../src/qgsdataprovider.moc.cpp		\
		../../src/qgsrasterdataprovider.moc.cpp		\
		qgswmsprovider.cpp				\
		qgswmsprovider.moc.cpp				\
		qgswmsprovider.h				\
		qgshttptransaction.cpp				\
		qgshttptransaction.moc.cpp			\
		qgshttptransaction.h

BUILT_SOURCES = $(wmsprovider_MOC)
                                
wmsprovider_la_LIBADD = $(QT_LDADD)
wmsprovider_la_LDFLAGS = -avoid-version -module
wmsprovider_la_CXXFLAGS = $(EXTRA_CXXFLAGS) $(QT_CXXFLAGS) $(DEBUG_QGIS)

CLEANFILES = $(BUILT_SOURCES)
