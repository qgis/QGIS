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
		qgisapp.h \
		main.cpp \
		qgisiface.cpp \
		qgisiface.h \
		qgisinterface.cpp \
		qgisinterface.h \
		qgsattributetable.cpp \
		qgsattributetable.h \
		qgsattributetabledisplay.cpp \
		qgsattributetabledisplay.h \
		qgscontcoldialog.cpp\
		qgscontinuouscolrenderer.cpp \
		qgscoordinatetransform.cpp \
		qgscoordinatetransform.h \
		qgscustomsymbol.cpp \
		qgscustomsymbol.h \
		qgsdatasource.cpp \
		qgsdatasource.h \
		qgsgraduatedsymrenderer.cpp \
		qgsgrasydialog.cpp \
		qgsgrasyextensionwidget.cpp \
		qgshelpviewer.cpp \
		qgshelpviewer.h \
		qgsidentifyresults.cpp \
		qgsidentifyresults.h \
		qgslayerproperties.cpp \
		qgslayerproperties.h \
		qgsrasterlayerproperties.cpp \
		qgsrasterlayerproperties.h \
		qgslegend.cpp \
		qgslegend.h \
		qgslegenditem.cpp \
		qgslegendview.cpp \
		qgslegendview.h \
		qgslinestyledialog.cpp \
		qgslinesymbol.cpp \
		qgslinesymbol.h \
		qgsmapcanvas.cpp \
		qgsmapcanvas.h \
		qgsmaplayer.cpp \
		qgsmaplayer.h \
		qgsvectorlayer.cpp \
		qgsmarkersymbol.cpp \
		qgsmarkersymbol.h \
		qgspatterndialog.cpp \
		qgspluginitem.cpp \
		qgspluginitem.h \
		qgspluginmanager.cpp \
		qgspluginmanager.h \
		qgspoint.cpp \
		qgspoint.h \
		qgspolygonsymbol.cpp \
		qgspolygonsymbol.h \
		qgsprojectio.cpp \
		qgsprojectio.h \
		qgsrangerenderitem.cpp \
		qgsrasterlayer.cpp \
		qgsrasterlayer.h \
		qgsrect.cpp \
		qgsrect.h \
		qgsrenderitem.cpp \
		qgsrenderitem.h \
		qgssinglesymrenderer.cpp \
		qgssisydialog.cpp \
		qgssymbol.cpp \
		qgssymbol.h \
		qgsmapserverexport.cpp \
		qgsmapserverexport.h \
		splashscreen.h \
		splashscreen.cpp \
		qgssymbologyutils.cpp\
		qgssymbologyutils.h \
		qgsvectorlayerproperties.cpp\
		qgsvectorlayerproperties.h \
		qgsfeature.cpp \
		qgsfeature.h \
		qgsfeatureattribute.cpp \
		qgsfeatureattribute.h \
		qgsfield.cpp \
		qgsfield.h \
		qgsproviderregistry.cpp \
		qgsproviderregistry.h \
		qgsprovidermetadata.cpp \
		qgsprovidermetadata.h \
		qgsdlgvectorlayerproperties.cpp \
		qgsdlgvectorlayerproperties.h \
		$(qgis_UI) \
		$(qgis_MOC)
qgis_MOC = 	qgisappbase.moc.cpp \
          	qgisapp.moc.cpp\
						qgisinterface.moc.cpp\
						qgsabout.moc.cpp\
          	qgsattributetable.moc.cpp \
          	qgsattributetablebase.moc.cpp\
						qgscontcoldialog.moc.cpp \
						qgscontcoldialogbase.moc.cpp\
						qgsgrasydialog.moc.cpp \
						qgsgrasydialogbase.moc.cpp \
						qgsgrasyextensionwidget.moc.cpp\
						qgshelpviewer.moc.cpp \
						qgsidentifyresultsbase.moc.cpp \
						qgslayerpropertiesbase.moc.cpp \
						qgslegend.moc.cpp \
						qgslegenditem.moc.cpp\
						qgslegenditembase.moc.cpp\
						qgslegendview.moc.cpp \
						qgslinestyledialog.moc.cpp \
						qgslinestyledialogbase.moc.cpp \
						qgsmapcanvas.moc.cpp \
						qgsmaplayer.moc.cpp \
						qgsmaplayerinterface.moc.cpp \
						qgsmapserverexport.moc.cpp \
						qgsmapserverexportbase.moc.cpp \
						qgsmessageviewer.moc.cpp \
						qgspatterndialog.moc.cpp \
						qgspatterndialogbase.moc.cpp \
						qgspluginmanager.moc.cpp \
						qgspluginmanagerbase.moc.cpp \
						qgsrasterlayer.moc.cpp \
						qgsrasterlayerproperties.moc.cpp \
						qgsrasterlayerpropertiesbase.moc.cpp\
						qgssisydialog.moc.cpp \
						qgssisydialogbase.moc.cpp \
						qgsvectorlayer.moc.cpp \
						qgsvectorlayerproperties.moc.cpp\
						qgsvectorlayerpropertiesbase.moc.cpp \
						splashscreen.moc.cpp \
						qgsdlgvectorlayerpropertiesbase.moc.cpp \
						qgsdlgvectorlayerproperties.moc.cpp \
						qgshelpviewerbase.moc.cpp
					
qgis_UI = qgisappbase.cpp \
          qgisappbase.h \
          qgsabout.cpp \
          qgsabout.h \
          qgsattributetablebase.cpp \
          qgsattributetablebase.h \
          qgscontcoldialogbase.cpp \
          qgscontcoldialogbase.h \
          qgsgrasydialogbase.cpp \
          qgsgrasydialogbase.h \
          qgshelpviewerbase.cpp \
          qgshelpviewerbase.h \
          qgsidentifyresultsbase.cpp \
          qgsidentifyresultsbase.h \
          qgslayerpropertiesbase.cpp \
          qgslayerpropertiesbase.h \
          qgslegenditembase.cpp \
          qgslegenditembase.h \
          qgslinestyledialogbase.cpp \
          qgslinestyledialogbase.h \
          qgsmapserverexportbase.cpp \
          qgsmapserverexportbase.h \
          qgsmessageviewer.cpp \
          qgsmessageviewer.h \
          qgspatterndialogbase.cpp \
          qgspatterndialogbase.h \
          qgspluginmanagerbase.cpp \
          qgspluginmanagerbase.h \
          qgsrasterlayerpropertiesbase.cpp\
          qgsrasterlayerpropertiesbase.h \
          qgssisydialogbase.cpp \
          qgssisydialogbase.h \
          qgsvectorlayerpropertiesbase.cpp \
          qgsvectorlayerpropertiesbase.h \
          qgsdlgvectorlayerpropertiesbase.cpp \
          qgsdlgvectorlayerpropertiesbase.h
qgis_UIC = qgisappbase.ui \
          qgsabout.ui \
          qgsattributetablebase.ui \
          qgshelpviewerbase.ui \
          qgsidentifyresultsbase.ui \
          qgslayerpropertiesbase.ui \
          qgslegenditembase.ui \
          qgsmapserverexportbase.ui \
          qgsmessageviewer.ui \
          qgspatterndialogbase.h \
          qgsrasterlayerpropertiesbase.ui \
          qgsdlgvectorlayerpropertiesbase.ui
# UI dependencies			
gsattributetablebase.cpp: gsattributetablebase.h qgsattributetablebase.ui
gsattributetablebase.h: qgsattributetablebase.ui
qgisappbase.cpp: qgisappbase.ui qgisappbase.h
qgisappbase.h: qgisappbase.ui
qgsabout.cpp: qgsabout.h qgsabout.ui
qgsabout.h: qgsabout.ui
qgscontcoldialogbase.cpp: qgscontcoldialogbase.h qgscontcoldialogbase.ui
qgscontcoldialogbase.h: qgscontcoldialogbase.ui
qgsgrasydialogbase.cpp: qgsgrasydialogbase.h qgsgrasydialogbase.ui
qgsgrasydialogbase.h: qgsgrasydialogbase.ui
qgshelpviewerbase.cpp: qgshelpviewerbase.h qgshelpviewerbase.ui 
qgshelpviewerbase.h: qgshelpviewerbase.ui
qgsidentifyresultsbase.cpp: qgsidentifyresultsbase.h qgsidentifyresultsbase.ui
qgsidentifyresultsbase.h: qgsidentifyresultsbase.ui
qgslayerpropertiesbase.cpp: qgslayerpropertiesbase.h qgslayerpropertiesbase.ui
qgslayerpropertiesbase.h: qgslayerpropertiesbase.ui
qgslegenditembase.cpp: qgslegenditembase.h qgslegenditembase.ui
qgslegenditembase.h: qgslegenditembase.ui
qgslinestyledialogbase.cpp: qgslinestyledialogbase.h qgslinestyledialogbase.ui
qgslinestyledialogbase.h: qgslinestyledialogbase.ui
qgsmapserverexportbase.cpp: qgsmapserverexportbase.h qgsmapserverexportbase.ui
qgsmapserverexportbase.h: qgsmapserverexportbase.ui
qgsmessageviewer.cpp: qgsmessageviewer.h qgsmessageviewer.ui
qgsmessageviewer.h: qgsmessageviewer.ui
qgspatterndialogbase.cpp: qgspatterndialogbase.h qgspatterndialogbase.ui
qgspatterndialogbase.h: qgspatterndialogbase.ui
qgspluginmanagerbase.cpp: qgspluginmanagerbase.h qgspluginmanagerbase.ui
qgspluginmanagerbase.h: qgspluginmanagerbase.ui
qgsrasterlayerpropertiesbase.cpp: qgsrasterlayerpropertiesbase.h qgsrasterlayerpropertiesbase.ui
qgsrasterlayerpropertiesbase.h: qgsrasterlayerpropertiesbase.ui
qgssisydialogbase.cpp: qgssisydialogbase.h qgssisydialogbase.ui
qgssisydialogbase.h: qgssisydialogbase.ui
qgsvectorlayerpropertiesbase.cpp: qgsvectorlayerpropertiesbase.h qgsvectorlayerpropertiesbase.ui
qgsvectorlayerpropertiesbase.h: qgsvectorlayerpropertiesbase.ui
qgsdlgvectorlayerpropertiesbase.cpp: qgsdlgvectorlayerpropertiesbase.h qgsdlgvectorlayerpropertiesbase.ui
qgsdlgvectorlayerpropertiesbase.h: qgsdlgvectorlayerpropertiesbase.ui

#PostgreSQL UI dependencies
qgsdbsourceselectbase.h: qgsdbsourceselectbase.ui
qgsdbsourceselectbase.cpp: qgsdbsourceselectbase.h qgsdbsourceselectbase.ui
qgsnewconnectionbase.h: qgsnewconnectionbase.ui
qgsnewconnectionbase.cpp: qgsnewconnectionbase.h qgsnewconnectionbase.ui

#PostgreSQL files
postgresSOURCES = qgsdbsourceselect.cpp \
									qgsnewconnection.cpp\
postgresUI = 	qgsdbsourceselectbase.h \
							qgsdbsourceselectbase.cpp\
							qgsnewconnectionbase.h\
							qgsnewconnectionbase.cpp
postgresUIC = qgsdbsourceselectbase.ui \
							qgsnewconnectionbase.ui	
postgresMOC = 	qgsdbsourceselectbase.moc.cpp \
								qgsdbsourceselect.moc.cpp \
								qgsnewconnection.moc.cpp\
								qgsnewconnectionbase.moc.cpp

# if PostgreSQL support requested, add PostgreSQL files				
if POSTGRESDB
qgis_SOURCES += $(postgresSOURCES)
qgis_UI += $(postgresUI) 
qgis_MOC += $(postgresMOC)
# add required defines for compiling PostgreSQL
PGDB = -DPOSTGRESQL -DHAVE_NAMESPACE_STD -DHAVE_CXX_STRING_HEADER -DDLLIMPORT=""
endif
PREFIX=-DPREFIX=\"$(prefix)\"
BUILT_SOURCES = $(qgis_MOC) $(qgis_UI)
qgis_LDADD := $(LDADD) $(QT_LDADD) $(GDAL_LIB) $(PG_LIB)
AM_CXXFLAGS := $(PREFIX) $(PGDB) $(GDAL_CFLAGS) $(CXXFLAGS) $(EXTRA_CXXFLAGS) $(QT_CXXFLAGS) -I$(PG_INC) $(DEBUG_QGIS)
CLEANFILES = $(BUILT_SOURCES)
