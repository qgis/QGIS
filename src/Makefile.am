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

headers =  qgis.h				\
		qgisiface.h			\
		qgisinterface.h			\
		qgsattributetable.h		\
		qgsattributetabledisplay.h	\
		qgscontcoldialog.h		\
		qgscontinuouscolrenderer.h	\
		qgscoordinatetransform.h	\
		qgscustomsymbol.h		\
		qgsdatabaselayer.h		\
		qgsdataprovider.h		\
		qgsdatasource.h			\
		qgsdlgvectorlayerproperties.h	\
		qgsfeature.h			\
		qgsfeatureattribute.h		\
		qgsfield.h			\
		qgsgraduatedsymrenderer.h	\
		qgsgrasydialog.h		\
		qgsgrasyextensionwidget.h	\
		qgshelpviewer.cpp		\
		qgshelpviewer.h			\
		qgsidentifyresults.h		\
		qgslegend.h			\
		qgslegenditem.h			\
		qgslinestyledialog.h		\
		qgslinesymbol.h			\
		qgsmapcanvas.h			\
		qgsmaplayer.h			\
		qgsmaplayerinterface.h		\
		qgsmapserverexport.h		\
		qgsmarkersymbol.h		\
		qgspatterndialog.h		\
		qgspluginitem.h			\
		qgspluginmanager.h		\
		qgspluginmetadata.h		\
		qgspluginregistry.h		\
		qgspoint.h			\
		qgspolygonsymbol.h		\
		qgsprojectio.h			\
		qgsprovidermetadata.h		\
		qgsproviderregistry.h		\
		qgsrangerenderitem.h		\
		qgsrasterlayer.h		\
		qgsrasterlayerproperties.h	\
		qgsrect.h			\
		qgsrenderer.h			\
		qgsrenderitem.h			\
		qgssinglesymrenderer.h		\
		qgssisydialog.h			\
		qgssymbol.h			\
		qgssymbologyutils.h		\
		qgsvectorlayer.h		\
		qgsvectorlayerproperties.h	\
		splashscreen.h			\
	        qgisapp.h


qgis_SOURCES =  main.cpp			\
		qgisapp.cpp			\
		qgisiface.cpp			\
		qgisinterface.cpp		\
		qgsattributetable.cpp		\
		qgsattributetabledisplay.cpp	\
		qgscontcoldialog.cpp		\
		qgscontinuouscolrenderer.cpp	\
		qgscoordinatetransform.cpp	\
		qgscustomsymbol.cpp		\
		qgsdatasource.cpp		\
		qgsdlgvectorlayerproperties.cpp	\
		qgsfeature.cpp			\
		qgsfeatureattribute.cpp		\
		qgsfield.cpp			\
		qgsgraduatedsymrenderer.cpp	\
		qgsgrasydialog.cpp		\
		qgsgrasyextensionwidget.cpp	\
		qgshelpviewer.cpp		\
		qgsidentifyresults.cpp		\
		qgslegend.cpp			\
		qgslegenditem.cpp		\
		qgslinestyledialog.cpp		\
		qgslinesymbol.cpp		\
		qgsmapcanvas.cpp		\
		qgsmaplayer.cpp			\
		qgsmapserverexport.cpp		\
		qgsmarkersymbol.cpp		\
		qgspatterndialog.cpp		\
		qgspluginitem.cpp		\
		qgspluginmanager.cpp		\
		qgspluginmetadata.cpp		\
		qgspluginregistry.cpp		\
		qgspoint.cpp			\
		qgspolygonsymbol.cpp		\
		qgsprojectio.cpp		\
		qgsprovidermetadata.cpp		\
		qgsproviderregistry.cpp		\
		qgsrangerenderitem.cpp		\
		qgsrasterlayer.cpp		\
		qgsrasterlayerproperties.cpp	\
		qgsrect.cpp			\
		qgsrenderitem.cpp		\
		qgssinglesymrenderer.cpp	\
		qgssisydialog.cpp		\
		qgssymbol.cpp			\
		qgssymbologyutils.cpp		\
		qgsvectorlayer.cpp		\
		qgsvectorlayerproperties.cpp	\
		splashscreen.cpp		\
		$(qgis_UI)

nodist_qgis_SOURCES	=	$(qgis_MOC)

qgis_MOC = 	qgisapp.moc.cpp				\
		qgisappbase.moc.cpp			\
		qgisinterface.moc.cpp			\
		qgsabout.moc.cpp			\
		qgsattributetable.moc.cpp		\
		qgsattributetablebase.moc.cpp		\
		qgscontcoldialog.moc.cpp		\
		qgscontcoldialogbase.moc.cpp		\
		qgsdlgvectorlayerproperties.moc.cpp	\
		qgsdlgvectorlayerpropertiesbase.moc.cpp	\
		qgsgrasydialog.moc.cpp			\
		qgsgrasydialogbase.moc.cpp		\
		qgsgrasyextensionwidget.moc.cpp		\
		qgshelpviewer.moc.cpp			\
		qgshelpviewerbase.moc.cpp		\
		qgsidentifyresultsbase.moc.cpp		\
		qgslegend.moc.cpp			\
		qgslegenditem.moc.cpp			\
		qgslegenditembase.moc.cpp		\
		qgslinestyledialog.moc.cpp		\
		qgslinestyledialogbase.moc.cpp		\
		qgsmapcanvas.moc.cpp			\
		qgsmaplayer.moc.cpp			\
		qgsmaplayerinterface.moc.cpp		\
		qgsmapserverexport.moc.cpp		\
		qgsmapserverexportbase.moc.cpp		\
		qgsmessageviewer.moc.cpp		\
		qgsoptionsbase.moc.cpp			\
		qgspatterndialog.moc.cpp		\
		qgspatterndialogbase.moc.cpp		\
		qgspluginmanager.moc.cpp		\
		qgspluginmanagerbase.moc.cpp		\
		qgsrasterlayer.moc.cpp			\
		qgsrasterlayerproperties.moc.cpp	\
		qgsrasterlayerpropertiesbase.moc.cpp	\
		qgssisydialog.moc.cpp			\
		qgssisydialogbase.moc.cpp		\
		qgsvectorlayer.moc.cpp			\
		qgsvectorlayerproperties.moc.cpp	\
		qgsvectorlayerpropertiesbase.moc.cpp 	\
		splashscreen.moc.cpp

qgis_UIHEADERS = \
          qgisappbase.h				\
          qgsabout.h				\
          qgsattributetablebase.h		\
          qgscontcoldialogbase.h		\
          qgsgrasydialogbase.h			\
          qgshelpviewerbase.cpp			\
          qgshelpviewerbase.h			\
          qgsidentifyresultsbase.h		\
          qgslegenditembase.h			\
          qgslinestyledialogbase.h		\
          qgsmapserverexportbase.h		\
          qgsmessageviewer.h			\
          qgspatterndialogbase.h		\
          qgspluginmanagerbase.h		\
          qgsrasterlayerpropertiesbase.h	\
          qgssisydialogbase.h			\
          qgsdlgvectorlayerpropertiesbase.h	\
          qgsoptionsbase.h			\
          qgsvectorlayerpropertiesbase.h

qgis_UI = qgisappbase.cpp			\
          qgsabout.cpp				\
          qgsattributetablebase.cpp		\
          qgscontcoldialogbase.cpp		\
          qgsgrasydialogbase.cpp		\
          qgshelpviewerbase.cpp			\
          qgsidentifyresultsbase.cpp		\
          qgslegenditembase.cpp			\
          qgslinestyledialogbase.cpp		\
          qgsmapserverexportbase.cpp		\
          qgsmessageviewer.cpp			\
          qgspatterndialogbase.cpp		\
          qgspluginmanagerbase.cpp		\
          qgsrasterlayerpropertiesbase.cpp	\
          qgssisydialogbase.cpp			\
          qgsdlgvectorlayerpropertiesbase.cpp	\
          qgsoptionsbase.cpp			\
          qgsvectorlayerpropertiesbase.cpp

qgis_UIC = 	qgisappbase.ui					\
		qgisappbase.ui.h				\
		qgsabout.ui					\
		qgsabout.ui.h					\
		qgsattributetablebase.ui			\
		qgsattributetablebase.ui.h			\
		qgscontcoldialogbase.ui				\
		qgsdbsourceselectbase.ui			\
		qgsdbsourceselectbase.ui.h			\
		qgsdlgvectorlayerpropertiesbase.ui		\
		qgsgrasydialogbase.ui				\
		qgshelpviewerbase.ui				\
		qgshelpviewerbase.ui.h				\
		qgsidentifyresultsbase.ui			\
		qgslegenditembase.ui				\
		qgslinestyledialogbase.ui			\
		qgsmapserverexportbase.ui			\
		qgsmapserverexportbase.ui.h			\
		qgsmessageviewer.ui				\
		qgsmessageviewer.ui.h				\
		qgsoptionsbase.ui				\
		qgsoptionsbase.ui.h				\
		qgspatterndialogbase.ui				\
		qgspluginmanagerbase.ui				\
		qgsrasterlayerpropertiesbase.ui			\
		qgsrasterlayerpropertiesbase.ui.h		\
		qgssisydialogbase.ui				\
		qgsvectorlayerpropertiesbase.ui

# XXX Actually the general pattern matching rules should already do these dependencies
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
postgresHEADERS = qgsnewconnection.h qgsdbsourceselect.h

postgresSOURCES = qgsdbsourceselect.cpp qgsnewconnection.cpp

postgresUIHEADERS = qgsdbsourceselectbase.h qgsnewconnectionbase.h

postgresUI = 	qgsdbsourceselectbase.cpp qgsnewconnectionbase.cpp

postgresUIC =   qgsdbsourceselectbase.ui qgsnewconnectionbase.ui	

postgresMOC =   qgsdbsourceselect.moc.cpp	\
		qgsdbsourceselectbase.moc.cpp	\
		qgsnewconnection.moc.cpp	\
		qgsnewconnectionbase.moc.cpp

# if PostgreSQL support requested, add PostgreSQL files				
if HAVE_POSTGRESQL
qgis_SOURCES += $(postgresSOURCES)
qgis_UI += $(postgresUI) 
qgis_MOC += $(postgresMOC)
# add required defines for compiling PostgreSQL
#PGDB = -DPOSTGRESQL -DHAVE_NAMESPACE_STD -DHAVE_CXX_STRING_HEADER -DDLLIMPORT=""
endif

PREFIX=-DPREFIX=\"$(prefix)\"
BUILT_SOURCES = $(qgis_MOC) $(qgis_UI) $(postgresMOC) $(postgresUI)
qgis_LDADD = $(LDADD) $(QT_LDADD) $(GDAL_LIB) $(PG_LIB)
AM_CXXFLAGS = $(PREFIX) $(PGDB) $(GDAL_CFLAGS) $(CXXFLAGS) $(EXTRA_CXXFLAGS) $(QT_CXXFLAGS) -I$(PG_INC) $(DEBUG_QGIS)
CLEANFILES = $(BUILT_SOURCES)

EXTRA_DIST = $(postgresUIC) $(qgis_UIC) xpm/*.png xpm/*.xpm xpm/16x16/mapserver.xpm xpm/16x16/camera.png
