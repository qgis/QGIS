# Copyright (C) 2003 Gary Sherman <sherman at mrcc.com>
#  
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without 
# modifications, as long as this notice is preserved.
# 
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
bin_PROGRAMS = qgis
%.moc.cpp: %.h
	$(MOC) -o $@ $<

%.h: %.ui
	$(UIC) -o $@ $<

%.cpp: %.ui
	$(UIC) -o $@ -impl $*.h $<
	
qgis_SOURCES =  qgisapp.cpp \
				main.cpp \
				qgisiface.cpp \
				qgisinterface.cpp \
				qgsattributetable.cpp \
				qgsattributetabledisplay.cpp \
				qgscoordinatetransform.cpp \
				qgscustomsymbol.cpp \
				qgsdatasource.cpp \
				qgsidentifyresults.cpp \
				qgslayerproperties.cpp \
				qgslegend.cpp \
				qgslegenditem.cpp \
				qgslinesymbol.cpp \
				qgsmapcanvas.cpp \
				qgsmaplayer.cpp \
				qgsmarkersymbol.cpp \
				qgspluginitem.cpp \
				qgspluginmanager.cpp \
				qgspoint.cpp \
				qgspolygonsymbol.cpp \
				qgsprojectio.cpp \
				qgsrasterlayer.cpp \
				qgsrect.cpp \
				qgsrenderer.cpp \
				qgsrenderitem.cpp \
				qgsshapefilelayer.cpp \
				qgssymbol.cpp\
				$(qgis_UI)\
				$(qgis_MOC)
qgis_MOC = 	qgisappbase.moc.cpp \
			qgisapp.moc.cpp\
			qgisinterface.moc.cpp\
			qgsabout.moc.cpp\
			qgsattributetablebase.moc.cpp\
			qgsattributetable.moc.cpp\
			qgsidentifyresultsbase.moc.cpp\
			qgslayerpropertiesbase.moc.cpp\
			qgslegend.moc.cpp\
			qgslegenditembase.moc.cpp\
			qgslegenditem.moc.cpp\
			qgsmapcanvas.moc.cpp\
			qgsmaplayer.moc.cpp\
			qgsmaplayerinterface.moc.cpp\
			qgsmessageviewer.moc.cpp\
			qgspluginmanagerbase.moc.cpp\
			qgsrasterlayer.moc.cpp\
			qgsshapefilelayer.moc.cpp
qgis_UI = qgisappbase.h\
			qgisappbase.cpp\
			qgslegenditembase.h\
			qgslegenditembase.cpp\
			qgsabout.h\
			qgsabout.cpp\
			qgslayerpropertiesbase.h\
			qgslayerpropertiesbase.cpp\
			qgsidentifyresultsbase.h\
			qgsidentifyresultsbase.cpp\
			qgsattributetablebase.h\
			qgsattributetablebase.cpp\
			qgspluginmanagerbase.h\
			qgspluginmanagerbase.cpp\
			qgsmessageviewer.h\
			qgsmessageviewer.cpp

# UI dependencies			
qgisappbase.h: qgisappbase.ui
qgisappbase.cpp: qgisappbase.ui qgisappbase.h
qgslegenditembase.h: qgslegenditembase.ui
qgslegenditembase.cpp: qgslegenditembase.h qgslegenditembase.ui
qgslayerpropertiesbase.h: qgslayerpropertiesbase.ui
qgslayerpropertiesbase.cpp: qgslayerpropertiesbase.h qgslayerpropertiesbase.ui
qgsabout.h: qgsabout.ui
qgsabout.cpp: qgsabout.h qgsabout.ui
qgsidentifyresultsbase.h: qgsidentifyresultsbase.ui
qgsidentifyresultsbase.cpp: qgsidentifyresultsbase.h qgsidentifyresultsbase.ui
gsattributetablebase.h: qgsattributetablebase.ui
gsattributetablebase.cpp: gsattributetablebase.h qgsattributetablebase.ui
qgspluginmanagerbase.h: qgspluginmanagerbase.ui
qgspluginmanagerbase.cpp: qgspluginmanagerbase.h qgspluginmanagerbase.ui
qgsmessageviewer.h: qgsmessageviewer.ui
qgsmessageviewer.cpp: qgsmessageviewer.h qgsmessageviewer.ui

#PostgreSQL UI dependencies
qgsdbsourceselectbase.h: qgsdbsourceselectbase.ui
qgsdbsourceselectbase.cpp: qgsdbsourceselectbase.h qgsdbsourceselectbase.ui
qgsnewconnectionbase.h: qgsnewconnectionbase.ui
qgsnewconnectionbase.cpp: qgsnewconnectionbase.h qgsnewconnectionbase.ui

#PostgreSQL files
postgres_SOURCES = 	qgsdbsourceselect.cpp \
 					qgsnewconnection.cpp\
					qgsdatabaselayer.cpp
postgres_UI = 	qgsdbsourceselectbase.h \
				qgsdbsourceselectbase.cpp\
				qgsnewconnectionbase.h\
				qgsnewconnectionbase.cpp
postgres_MOC = 	qgsdbsourceselectbase.moc.cpp \
	 			qgsdatabaselayer.moc.cpp \
				qgsdbsourceselect.moc.cpp \
				qgsnewconnection.moc.cpp\
				qgsnewconnectionbase.moc.cpp
				
# if PostgreSQL support requested, add PostgreSQL files				
if POSTGRESDB
qgis_SOURCES += $(postgres_SOURCES)
					
qgis_UI += $(postgres_UI) 
qgis_MOC += $(postgres_MOC)
# add required defines for compiling PostgreSQL
PGDB = -DPOSTGRESQL -DHAVE_NAMESPACE_STD -DHAVE_CXX_STRING_HEADER -DDLLIMPORT=""
endif

BUILT_SOURCES = $(qgis_MOC) $(qgis_UI)
qgis_LDADD := $(LDADD) $(QT_LDADD) $(GDAL_LIB) $(PG_LIB)
AM_CXXFLAGS := $(PGDB) $(CXXFLAGS) $(EXTRA_CXXFLAGS) -I../include $(QT_CXXFLAGS) -I$(PG_INC)
CLEANFILES = $(BUILT_SOURCES)
