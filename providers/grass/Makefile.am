plugindir = ${pkglibdir}
plugin_LTLIBRARIES = grassprovider.la 

%.moc.cpp: %.h
	$(MOC) -o $@ $<

%.h: %.ui
	$(UIC) -o $@ $<

%.cpp: %.ui
	$(UIC) -o $@ -impl $*.h $<

grassprovider_la_SOURCES = qgsgrassprovider.cpp \
	                qgsgrassprovider.h \
			qgsgrass.cpp \
                        qgsgrass.h \
		        ../../src/qgsfeature.cpp \
			../../src/qgsfeatureattribute.cpp \
			../../src/qgsrect.cpp \
			../../src/qgspoint.cpp \
			../../src/qgsfield.cpp 

grassprovider_la_LIBADD = $(QT_LDADD)  $(GRASS_LIB)
grassprovider_la_LDFLAGS = -avoid-version -module
grassprovider_la_CXXFLAGS = $(CXXFLAGS) -I$(GRASS_INC) $(EXTRA_CXXFLAGS) $(QT_CXXFLAGS) $(DEBUG_QGIS) 
