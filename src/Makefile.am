# Copyright (C) 2003 Gary Sherman <sherman at mrcc.com>
#  
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without 
# modifications, as long as this notice is preserved.
# 
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

SUBDIRS = svg

PREFIX=-DPREFIX=\"$(prefix)\"
PLUGINPATH=-DPLUGINPATH=\"$(pkglibdir)\"
PKGDATAPATH=-DPKGDATAPATH=\"$(pkgdatadir)\"

%.moc.cpp: %.h
	$(MOC) -o $@ $<

%.moc.uic.cpp: %.uic.h
	$(MOC) -o $@ $<

%.uic.h: %.ui 
	$(UIC) -o $@ $<

%.uic.cpp: %.ui
	$(UIC) -o $@ -impl $*.uic.h $<


## main app
bin_PROGRAMS = qgis

## ancillary library for plug-ins
lib_LTLIBRARIES = libqgis.la

headers =  	qgis.h					\
	   	qgisiface.h				\
	   	qgisinterface.h				\
	   	qgsattributetable.h			\
	   	qgsattributetabledisplay.h		\
	   	qgscontcoldialog.h			\
	   	qgscontinuouscolrenderer.h		\
	   	qgscoordinatetransform.h		\
	   	qgscustomsymbol.h			\
	   	qgsdatabaselayer.h			\
	   	qgsdataprovider.h			\
	   	qgsdatasource.h				\
	   	qgsdlgvectorlayerproperties.h		\
	   	qgsfeature.h				\
	   	qgsfeatureattribute.h			\
	   	qgsfield.h				\
		qgsgraduatedmarenderer.h		\
	   	qgsgraduatedsymrenderer.h		\
		qgsgramadialog.h			\
		qgsgramaextensionwidget.h		\
	   	qgsgrasydialog.h			\
	   	qgsgrasyextensionwidget.h		\
	   	qgshelpviewer.h				\
	   	qgsidentifyresults.h			\
	   	qgslegend.h				\
	   	qgslegenditem.h				\
	   	qgslinestyledialog.h			\
	   	qgslinesymbol.h				\
	   	qgsmapcanvas.h				\
	   	qgsmaplayer.h				\
	   	qgsmaplayerinterface.h			\
		qgsmapserverexport.h			\
		qgsmarkerdialog.h			\
	   	qgsmarkersymbol.h			\
	   	qgspatterndialog.h			\
	   	qgspluginitem.h				\
	   	qgspluginmanager.h			\
	   	qgspluginmetadata.h			\
	   	qgspluginregistry.h			\
	   	qgspoint.h				\
	   	qgspolygonsymbol.h			\
	   	qgsprojectio.h				\
	   	qgsprovidermetadata.h			\
	   	qgsproviderregistry.h			\
	   	qgsrangerenderitem.h			\
	   	qgsrasterlayer.h			\
	   	qgsrasterlayerproperties.h		\
	   	qgsrect.h				\
	   	qgsrenderer.h				\
	   	qgsrenderitem.h				\
	   	qgssinglesymrenderer.h			\
	   	qgssimarenderer.h			\
	   	qgssimadialog.h				\
	   	qgssisydialog.h				\
	   	qgssymbol.h				\
	   	qgssymbologyutils.h			\
	   	qgsvectorlayer.h			\
	   	qgsvectorlayerproperties.h		\
	   	splashscreen.h				\
	   	qgisapp.h


## qt designer files for all the GUI stuff
qgis_UIC = 	qgisappbase.ui					\
		qgsabout.ui					\
		qgsattributetablebase.ui			\
		qgscontcoldialogbase.ui				\
		qgsdlgvectorlayerpropertiesbase.ui		\
		qgsgramadialogbase.ui				\
		qgsgrasydialogbase.ui				\
		qgshelpviewerbase.ui				\
		qgsidentifyresultsbase.ui			\
		qgslegenditembase.ui				\
		qgslinestyledialogbase.ui			\
		qgsmapserverexportbase.ui			\
		qgsmarkerdialogbase.ui				\
		qgsmessageviewer.ui				\
		qgsoptionsbase.ui				\
		qgspatterndialogbase.ui				\
		qgspluginmanagerbase.ui				\
		qgsrasterlayerpropertiesbase.ui			\
		qgssimadialogbase.ui				\
		qgssisydialogbase.ui				\
		qgsvectorlayerpropertiesbase.ui

## all the ui files generate a corresponding header file
qgis_UIHEADERS = $(qgis_UIC:.ui=.uic.h)

## similarly all the ui files generate a corresponding implementation file
qgis_UISOURCES = $(qgis_UIC:.ui=.uic.cpp)

## these are optional ancillary user-defined qtdesigner-based headers
qgis_UIH = 	qgisappbase.ui.h				\
		qgsabout.ui.h					\
		qgsattributetablebase.ui.h			\
		qgshelpviewerbase.ui.h				\
		qgsmapserverexportbase.ui.h			\
		qgsmessageviewer.ui.h				\
		qgsoptionsbase.ui.h				\
		qgsrasterlayerpropertiesbase.ui.h

## these generated files need MOC files, too
qgis_UIMOC = $(qgis_UIC:.ui=.moc.uic.cpp)


## non-uic files generated from MOC
qgis_MOC = 	qgisapp.moc.cpp				\
		qgisinterface.moc.cpp			\
		qgsattributetable.moc.cpp		\
		qgscontcoldialog.moc.cpp		\
		qgsdlgvectorlayerproperties.moc.cpp	\
		qgsgramadialog.moc.cpp			\
		qgsgramaextensionwidget.moc.cpp		\
		qgsgrasydialog.moc.cpp			\
		qgsgrasyextensionwidget.moc.cpp		\
		qgshelpviewer.moc.cpp			\
		qgslegend.moc.cpp			\
		qgslegenditem.moc.cpp			\
		qgslinestyledialog.moc.cpp		\
		qgsmapcanvas.moc.cpp			\
		qgsmaplayer.moc.cpp			\
		qgsmaplayerinterface.moc.cpp		\
		qgsmapserverexport.moc.cpp		\
		qgsmarkerdialog.moc.cpp			\
		qgspatterndialog.moc.cpp		\
		qgspluginmanager.moc.cpp		\
		qgsrasterlayer.moc.cpp			\
		qgsrasterlayerproperties.moc.cpp	\
		qgssimadialog.moc.cpp			\
		qgssisydialog.moc.cpp			\
		qgsvectorlayer.moc.cpp			\
		qgsvectorlayerproperties.moc.cpp	\
		splashscreen.moc.cpp


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
		qgsgraduatedmarenderer.cpp	\
		qgsgraduatedsymrenderer.cpp	\
		qgsgramadialog.cpp		\
		qgsgramaextensionwidget.cpp	\
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
		qgsmarkerdialog.cpp		\
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
		qgssimadialog.cpp		\
		qgssimarenderer.cpp		\
		qgssinglesymrenderer.cpp	\
		qgssisydialog.cpp		\
		qgssymbol.cpp			\
		qgssymbologyutils.cpp		\
		qgsvectorlayer.cpp		\
		qgsvectorlayerproperties.cpp	\
		splashscreen.cpp

postgresHEADERS = qgsdbsourceselect.h qgsnewconnection.h
postgresSOURCES = qgsdbsourceselect.cpp qgsnewconnection.cpp
postgresUIC     = qgsdbsourceselectbase.ui qgsnewconnectionbase.ui	
postgresUIH     = 


if HAVE_POSTGRESQL
postgresUIHEADERS = $(postgresUIC:.ui=.uic.h)
postgresUISOURCES = $(postgresUIC:.ui=.uic.cpp)
postgresUIMOC     = $(postgresUIC:.ui=.moc.uic.cpp)

postgresMOC       = $(postgresHEADERS:.h=.moc.cpp)

qgis_SOURCES     += $(postgresSOURCES)
# qgis_MOC       += $(postgresMOC)
# qgis_UIHEADERS += $(postgresUIHEADERS) 
# qgis_UISOURCES += $(postgresUISOURCES) 
# qgis_UIMOC     += $(postgresUIMOC)

endif


nodist_qgis_SOURCES = $(qgis_MOC) $(qgis_UISOURCES) $(qgis_UIMOC) \
                      $(postgresMOC) $(postgresUISOURCES) $(postgresUIMOC)


BUILT_SOURCES = $(qgis_MOC) $(qgis_UIMOC) $(qgis_UIHEADERS) $(qgis_UISOURCES) \
	        $(postgresMOC) $(postgresUIMOC) $(postgresUIHEADERS) $(postgresUISOURCES)


qgis_LDADD = $(LDADD) $(QT_LDADD) $(GDAL_LDADD) $(PG_LIB)
qgis_CXXFLAGS = $(PREFIX) $(PLUGINPATH) $(PKGDATAPATH) $(GDAL_CFLAGS) $(CXXFLAGS) $(EXTRA_CXXFLAGS) $(QT_CXXFLAGS) -I$(PG_INC) $(DEBUG_QGIS)

libqgis_la_SOURCES = qgsmapcanvas.cpp qgsmapcanvas.moc.cpp qgssymbol.cpp qgsmaplayer.cpp qgsrasterlayer.cpp qgsvectorlayer.cpp qgsmaplayer.moc.cpp qgsrasterlayer.moc.cpp qgsvectorlayer.moc.cpp qgsfield.cpp qgsfeature.cpp qgsfeatureattribute.cpp qgsrect.cpp qgscoordinatetransform.cpp
libqgis_la_LIBFLAGS =  $(QT_LDADD)
libqgis_la_CXXFLAGS = $(PREFIX) $(PLUGINPATH) $(PKGDATAPATH) $(GDAL_CFLAGS) $(CXXFLAGS) $(EXTRA_CXXFLAGS) $(QT_CXXFLAGS) -I$(PG_INC) $(DEBUG_QGIS)

## for installing headers in $(includedir)/qgis
pkginclude_HEADERS = $(headers) $(qgis_UIHEADERS) $(qgis_UIH) $(postgresHEADERS) $(postgresUIH)
pkginclude_dir = $(includedir)/qgis



CLEANFILES = $(BUILT_SOURCES)

EXTRA_DIST = $(postgresHEADERS) $(postgresSOURCES) $(postgresUIC) $(qgis_UIC) $(qgis_UIH) $(postgresUIH) xpm/*.png xpm/*.xpm xpm/16x16/mapserver.xpm xpm/16x16/camera.png
