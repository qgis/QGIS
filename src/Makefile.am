# Copyright (C) 2003 Gary Sherman <sherman at mrcc.com>
#  
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without 
# modifications, as long as this notice is preserved.
# 
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

SUBDIRS = svg images themes resources

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

headers =	qgisapp.h					\
		qgsaddattrdialog.h \
		qgis.h						\
		qgisiface.h					\
		qgisinterface.h					\
		qgsacetatelines.h				\
		qgsacetateobject.h				\
		qgsacetaterectangle.h				\
		qgsattributeaction.h				\
		qgsattributeactiondialog.h			\
		qgsattributedialog.h				\
		qgsattributetabledisplay.h			\
		qgsattributetable.h				\
		qgsclipper.h					\
		qgscolortable.h					\
		qgscomposer.h					\
		qgscomposition.h				\
		qgscomposeritem.h				\
		qgscomposermap.h				\
		qgscomposervectorlegend.h			\
		qgscomposerlabel.h				\
		qgscomposerview.h				\
		qgscomposition.h				\
		qgscontcoldialog.h				\
		qgscontinuouscolrenderer.h			\
		qgscoordinatetransform.h			\
		qgsencodingfiledialog.h				\
		qgsmaptopixel.h					\
		qgscustomsymbol.h				\
		qgsdataprovider.h				\
		qgsdatasource.h					\
		qgsdatasourceuri.h				\
		qgsdelattrdialog.h				\
		qgsdlgvectorlayerproperties.h			\
	  	qgsexception.h					\
	  	qgscsexception.h				\
		qgsfeatureattribute.h				\
		qgsfeature.h					\
		qgsfield.h					\
		qgsgeomtypedialog.h				\
		qgsgraduatedmarenderer.h			\
		qgsgraduatedsymrenderer.h			\
		qgsgramadialog.h				\
		qgsgramaextensionwidget.h			\
		qgsgrasydialog.h				\
		qgshelpviewer.h					\
		qgsidentifyresults.h				\
		qgslabelattributes.h				\
		qgslabeldialog.h				\
		qgslabel.h					\
		qgslegend.h					\
		qgslegenditem.h					\
		qgsline.h					\
		qgslinestyledialog.h				\
		qgslinesymbol.h					\
		qgsludialog.h					\
		qgsmapcanvas.h					\
		qgsmaplayer.h					\
		qgsmaplayerinterface.h				\
		qgsmaplayerregistry.h				\
		qgsmapserverexport.h				\
		qgsmarkerdialog.h				\
		qgsmarkersymbol.h				\
		qgsmeasure.h					\
		qgsoptions.h					\
		qgspatterndialog.h				\
		qgspluginitem.h					\
		qgspluginmanager.h				\
		qgspluginmetadata.h				\
		qgspluginregistry.h				\
		qgspoint.h					\
		qgspolygonsymbol.h				\
		qgsproject.h					\
		qgsprojectproperty.h				\
		qgsprojectproperties.h				\
		qgsprovidercountcalcevent.h			\
		qgsproviderextentcalcevent.h			\
		qgsprovidermetadata.h				\
		qgsproviderregistry.h				\
		qgsrangerenderitem.h				\
		qgsrasterlayer.h				\
		qgsrasterlayerproperties.h			\
		qgsrect.h					\
		qgsrenderer.h					\
		qgsrenderitem.h					\
		qgsrunprocess.h					\
		qgsscalecalculator.h				\
		qgssimadialog.h					\
		qgssimarenderer.h				\
		qgssinglesymrenderer.h				\
		qgssisydialog.h					\
		qgssvgcache.h					\
		qgsmarkercatalogue.h				\
		qgssymbol.h					\
		qgssymbologyutils.h				\
		qgsuniquevalrenderer.h				\
		qgsuvaldialog.h					\
		qgsuvalmadialog.h				\
		qgsuvalmarenderer.h				\
		qgsvectordataprovider.h				\
		qgsvectorfilewriter.h				\
		qgsvectorlayer.h				\
		qgspointstylewidget.h				\
		qgslinestylewidget.h				\
		qgsfillstylewidget.h				\
		splashscreen.h				

## qt designer files for all the GUI stuff

qgis_UIC =	qgisappbase.ui				\
		qgsabout.ui				\
		qgsaddattrdialogbase.ui			\
		qgsattributedialogbase.ui		\
		qgsattributeactiondialogbase.ui		\
		qgsattributetablebase.ui		\
		qgscomposerbase.ui			\
		qgscompositionbase.ui			\
		qgscomposermapbase.ui			\
		qgscomposervectorlegendbase.ui		\
		qgscomposerlabelbase.ui		     	\
		qgscontcoldialogbase.ui			\
		qgsdelattrdialogbase.ui			\
		qgsdlgvectorlayerpropertiesbase.ui	\
		qgsgeomtypedialogbase.ui		\
		qgsgramadialogbase.ui			\
		qgsgrasydialogbase.ui			\
		qgshelpviewerbase.ui			\
		qgsidentifyresultsbase.ui		\
		qgslabeldialogbase.ui			\
		qgslegenditembase.ui			\
		qgslinestyledialogbase.ui		\
		qgsludialogbase.ui			\
		qgsmapserverexportbase.ui		\
		qgsmarkerdialogbase.ui			\
		qgsmessageviewer.ui			\
		qgsmeasurebase.ui			\
		qgsoptionsbase.ui			\
		qgspatterndialogbase.ui			\
		qgspluginmanagerbase.ui			\
		qgsprojectpropertiesbase.ui		\
		qgsrasterlayerpropertiesbase.ui		\
		qgssimadialogbase.ui			\
		qgssisydialogbase.ui			\
		qgsuvaldialogbase.ui			\
		qgsuvalmadialogbase.ui			\
		qgspointstylewidgetbase.ui		\
		qgsfillstylewidgetbase.ui		\
		qgslinestylewidgetbase.ui

## all the ui files generate a corresponding header file
qgis_UIHEADERS = $(qgis_UIC:.ui=.uic.h)

## similarly all the ui files generate a corresponding implementation file
qgis_UISOURCES = $(qgis_UIC:.ui=.uic.cpp)

## these are optional ancillary user-defined qtdesigner-based headers
qgis_UIH =	qgisappbase.ui.h			\
		qgsabout.ui.h				\
		qgsattributetablebase.ui.h		\
		qgshelpviewerbase.ui.h			\
		qgsmapserverexportbase.ui.h		\
		qgsmessageviewer.ui.h			\
		qgsoptionsbase.ui.h			\
		qgsrasterlayerpropertiesbase.ui.h	\
		qgslabeldialogbase.ui.h			\
		qgsprojectpropertiesbase.ui.h

## these generated files need MOC files, too
qgis_UIMOC = $(qgis_UIC:.ui=.moc.uic.cpp)


## non-uic files generated from MOC

qgis_MOC =	qgisapp.moc.cpp				\
		qgisinterface.moc.cpp			\
		qgsaddattrdialog.moc.cpp		\
		qgsattributeactiondialog.moc.cpp	\
		qgsattributedialog.moc.cpp		\
		qgsattributetable.moc.cpp		\
		qgscomposer.moc.cpp			\
		qgscomposition.moc.cpp			\
		qgscomposermap.moc.cpp			\
		qgscomposervectorlegend.moc.cpp		\
		qgscomposerlabel.moc.cpp		\
		qgscomposerview.moc.cpp			\
		qgsattributetabledisplay.moc.cpp		\
		qgscoordinatetransform.moc.cpp		\
		qgscontcoldialog.moc.cpp		\
		qgsdataprovider.moc.cpp		\
		qgsdelattrdialog.moc.cpp			\
		qgsdlgvectorlayerproperties.moc.cpp	\
		qgsencodingfiledialog.moc.cpp		\
		qgsgeomtypedialog.moc.cpp		\
		qgsgramadialog.moc.cpp			\
		qgsgramaextensionwidget.moc.cpp		\
		qgsgrasydialog.moc.cpp			\
		qgshelpviewer.moc.cpp			\
		qgsidentifyresults.moc.cpp		\
		qgslabeldialog.moc.cpp			\
		qgslinestyledialog.moc.cpp		\
		qgsludialog.moc.cpp			\
		qgsmapcanvas.moc.cpp			\
		qgsmaplayerinterface.moc.cpp		\
		qgsmaplayer.moc.cpp			\
		qgsmaplayerregistry.moc.cpp		\
		qgsmapserverexport.moc.cpp		\
		qgsmarkerdialog.moc.cpp			\
		qgsmeasure.moc.cpp			\
		qgsoptions.moc.cpp			\
		qgspatterndialog.moc.cpp		\
		qgspluginmanager.moc.cpp		\
		qgsprojectproperties.moc.cpp		\
		qgsrasterlayer.moc.cpp			\
		qgsrasterlayerproperties.moc.cpp	\
		qgsrunprocess.moc.cpp			\
		qgssimadialog.moc.cpp			\
		qgssisydialog.moc.cpp			\
		qgsuvaldialog.moc.cpp			\
		qgsuvalmadialog.moc.cpp			\
		qgsvectorlayer.moc.cpp			\
		qgsvectordataprovider.moc.cpp		\
		qgspointstylewidget.moc.cpp 		\
		qgsfillstylewidget.moc.cpp		\
		qgslinestylewidget.moc.cpp		\
		splashscreen.moc.cpp 


qgis_SOURCES =	main.cpp			\
		qgisapp.cpp			\
		qgisiface.cpp			\
		qgisinterface.cpp		\
		qgsacetatelines.cpp		\
		qgsacetateobject.cpp		\
		qgsacetaterectangle.cpp		\
		qgsaddattrdialog.cpp		\
		qgsattributeaction.cpp		\
		qgsattributeactiondialog.cpp	\
		qgsattributedialog.cpp		\
		qgsattributetable.cpp		\
		qgsattributetabledisplay.cpp	\
		qgsclipper.cpp			\
		qgscolortable.cpp		\
		qgscomposer.cpp			\
		qgscomposition.cpp		\
		qgscomposeritem.cpp		\
		qgscomposermap.cpp		\
		qgscomposervectorlegend.cpp	\
		qgscomposerlabel.cpp		\
		qgscomposerview.cpp		\
		qgscontcoldialog.cpp		\
		qgscontinuouscolrenderer.cpp	\
		qgscoordinatetransform.cpp	\
		qgsencodingfiledialog.cpp	\
		qgsmaptopixel.cpp		\
		qgscustomsymbol.cpp		\
		qgsdatasource.cpp		\
		qgsdelattrdialog.cpp		\
		qgsdlgvectorlayerproperties.cpp	\
		qgsfeatureattribute.cpp		\
		qgsfeature.cpp			\
		qgsfield.cpp			\
		qgsgeomtypedialog.cpp		\
		qgsrenderer.cpp 		\
		qgsgraduatedmarenderer.cpp	\
		qgsgraduatedsymrenderer.cpp	\
		qgsgramadialog.cpp		\
		qgsgramaextensionwidget.cpp	\
		qgsgrasydialog.cpp		\
		qgshelpviewer.cpp		\
		qgsidentifyresults.cpp		\
		qgslabelattributes.cpp		\
		qgslabel.cpp			\
		qgslabeldialog.cpp		\
		qgsline.cpp			\
		qgslinestyledialog.cpp		\
		qgslinesymbol.cpp		\
		qgsludialog.cpp			\
		qgsmapcanvas.cpp		\
		qgsmaplayer.cpp			\
		qgsmaplayerregistry.cpp		\
		qgsmapserverexport.cpp		\
		qgsmarkerdialog.cpp		\
		qgsmarkersymbol.cpp		\
		qgsmeasure.cpp			\
		qgsoptions.cpp			\
		qgspatterndialog.cpp		\
		qgspluginitem.cpp		\
		qgspluginmanager.cpp		\
		qgspluginmetadata.cpp		\
		qgspluginregistry.cpp		\
		qgspoint.cpp			\
		qgspolygonsymbol.cpp		\
		qgsproject.cpp			\
		qgsprojectproperties.cpp	\
		qgsprojectproperty.cpp		\
		qgsprovidercountcalcevent.cpp	\
		qgsproviderextentcalcevent.cpp	\
		qgsprovidermetadata.cpp		\
		qgsproviderregistry.cpp		\
		qgsrangerenderitem.cpp		\
		qgsrasterlayer.cpp		\
		qgsrasterlayerproperties.cpp	\
		qgsrect.cpp			\
		qgsrenderitem.cpp		\
		qgsrunprocess.cpp		\
		qgsscalecalculator.cpp		\
		qgssimadialog.cpp		\
		qgssimarenderer.cpp		\
		qgssinglesymrenderer.cpp	\
		qgssisydialog.cpp		\
		qgssvgcache.cpp			\
		qgsmarkercatalogue.cpp		\
		qgssymbol.cpp			\
		qgssymbologyutils.cpp		\
		qgsuniquevalrenderer.cpp	\
		qgsuvaldialog.cpp		\
		qgsuvalmadialog.cpp		\
		qgsuvalmarenderer.cpp		\
		qgsvectordataprovider.cpp	\
		qgsvectorfilewriter.cpp		\
		qgsvectorlayer.cpp		\
		qgspointstylewidget.cpp		\
		qgslinestylewidget.cpp		\
		qgsfillstylewidget.cpp		\
		splashscreen.cpp 

postgresHEADERS = qgsdbsourceselect.h		\
		  qgsnewconnection.h		\
		  qgspgquerybuilder.h		\
		  qgspgutil.h

postgresSOURCES = qgsdbsourceselect.cpp		\
		  qgsnewconnection.cpp		\
		  qgspgquerybuilder.cpp		\
		  qgspgutil.h

postgresUIC     = qgsdbsourceselectbase.ui		\
		  qgsnewconnectionbase.ui		\
		  qgspgquerybuilderbase.ui	

postgresUIH     = 
postgresLIBSOURCES = qgspgutil.cpp

if HAVE_POSTGRESQL
postgresUIHEADERS = $(postgresUIC:.ui=.uic.h)
postgresUISOURCES = $(postgresUIC:.ui=.uic.cpp)
postgresUIMOC     = $(postgresUIC:.ui=.moc.uic.cpp)

postgresMOC       = $(postgresHEADERS:.h=.moc.cpp)

qgis_SOURCES     += $(postgresSOURCES)
endif


nodist_qgis_SOURCES = $(qgis_MOC) $(qgis_UISOURCES) $(qgis_UIMOC) \
                      $(postgresMOC) $(postgresUISOURCES) $(postgresUIMOC)


BUILT_SOURCES = $(qgis_MOC) $(qgis_UIMOC) $(qgis_UIHEADERS) $(qgis_UISOURCES) \
					$(postgresMOC) $(postgresUIMOC) $(postgresUIHEADERS) $(postgresUISOURCES)


qgis_LDADD = $(LDADD) $(QT_LDADD) $(GDAL_LDADD) $(PG_LIB) $(GEOS_LDADD) libqgis.la
qgis_LDFLAGS = -rdynamic
qgis_CXXFLAGS = $(PREFIX) $(PLUGINPATH) $(PKGDATAPATH) $(GDAL_CFLAGS) $(CXXFLAGS) $(EXTRA_CXXFLAGS) $(QT_CXXFLAGS) -I$(PG_INC) $(DEBUG_QGIS) $(GEOS_CFLAGS)

libqgis_la_SOURCES =				\
		qgisapp.cpp			\
		qgisapp.moc.cpp			\
		qgisappbase.moc.uic.cpp		\
		qgisappbase.uic.cpp		\
		qgsacetateobject.cpp		\
		qgsacetaterectangle.cpp		\
		qgsattributeaction.cpp		\
		qgsclipper.cpp			\
		qgscolortable.cpp		\
		qgscomposer.cpp			\
		qgscomposer.moc.cpp		\
		qgscomposerbase.uic.cpp		\
		qgscomposerbase.moc.uic.cpp	\
		qgscomposeritem.cpp		\
		qgscomposerlabel.cpp		\
		qgscomposerlabel.moc.cpp	\
		qgscomposerlabelbase.uic.cpp	\
		qgscomposerlabelbase.moc.uic.cpp	\
		qgscomposermap.cpp		\
		qgscomposermap.moc.cpp		\
		qgscomposermapbase.uic.cpp	\
		qgscomposermapbase.moc.uic.cpp  \
		qgscompositionbase.uic.cpp	\
		qgscompositionbase.moc.uic.cpp	\
		qgscomposervectorlegend.cpp	\
		qgscomposervectorlegend.moc.cpp \
		qgscomposervectorlegendbase.uic.cpp \
		qgscomposervectorlegendbase.moc.uic.cpp \
		qgscomposerview.moc.cpp		\
		qgscomposerview.cpp		\
		qgscomposition.cpp		\
		qgscomposition.moc.cpp		\
		qgscontinuouscolrenderer.cpp	\
		qgscoordinatetransform.cpp	\
		qgscoordinatetransform.moc.cpp  \
		qgsdataprovider.moc.cpp         \
		qgsfeature.cpp			\
		qgsfeatureattribute.cpp		\
		qgsfield.cpp			\
		qgsgraduatedmarenderer.cpp	\
		qgsgraduatedsymrenderer.cpp	\
		qgslabel.cpp			\
		qgslegend.cpp			\
		qgslegend.moc.cpp			\
		qgslegenditem.cpp		\
		qgslegenditem.moc.cpp			\
		qgsmapcanvas.cpp		\
		qgsmapcanvas.moc.cpp		\
		qgsmaplayer.cpp			\
		qgsmaplayer.moc.cpp		\
		qgsmaplayerregistry.cpp		\
		qgsmaplayerregistry.moc.cpp	\
		qgsmaptopixel.cpp		\
		qgsmarkersymbol.cpp		\
		qgspoint.cpp			\
		qgspoint.moc.cpp		\
		qgsprojectproperty.cpp		\
		qgsprovidercountcalcevent.cpp	\
		qgsproviderextentcalcevent.cpp	\
		qgsproviderregistry.cpp		\
		qgsrangerenderitem.cpp		\
		qgsrasterlayer.cpp		\
		qgsrasterlayer.moc.cpp		\
		qgsrect.cpp			\
		qgsrenderer.cpp 		\
		qgsrenderitem.cpp		\
		qgsrunprocess.cpp		\
		qgsrunprocess.moc.cpp		\
		qgsscalecalculator.cpp		\
		qgssimarenderer.cpp		\
		qgssinglesymrenderer.cpp	\
		qgsmarkercatalogue.cpp		\
		qgssymbol.cpp			\
		qgsuniquevalrenderer.cpp	\
		qgsuvalmarenderer.cpp		\
		qgsvectordataprovider.cpp	\
		qgsvectordataprovider.moc.cpp	\
		qgsvectorfilewriter.cpp		\
		qgsvectorlayer.cpp		\
		qgsvectorlayer.moc.cpp 		\
		qgspointstylewidgetbase.uic.cpp	\
		qgspointstylewidgetbase.moc.uic.cpp	\
		qgslinestylewidgetbase.uic.cpp		\
		qgslinestylewidgetbase.moc.uic.cpp	\
		qgsfillstylewidgetbase.uic.cpp		\
		qgsfillstylewidgetbase.moc.uic.cpp	\
		qgspointstylewidget.cpp		\
		qgspointstylewidget.moc.cpp	\
		qgslinestylewidget.cpp		\
		qgslinestylewidget.moc.cpp	\
		qgsfillstylewidget.cpp		\
		qgsfillstylewidget.moc.cpp		

if HAVE_POSTGRESQL
libqgis_la_SOURCES += $(postgresLIBSOURCES)
endif

libqgis_la_LIBFLAGS = $(QT_LDADD) $(GEOS_LDADD)
libqgis_la_LDFLAGS = -version-info $(INTERFACE_VERSION)
libqgis_la_CXXFLAGS = $(PREFIX) $(PLUGINPATH) $(PKGDATAPATH) $(GDAL_CFLAGS) $(CXXFLAGS) $(EXTRA_CXXFLAGS) $(QT_CXXFLAGS) -I$(PG_INC) $(DEBUG_QGIS) $(GEOS_CFLAGS)


## for installing headers in $(includedir)/qgis
pkginclude_HEADERS = $(headers) $(qgis_UIHEADERS) $(qgis_UIH) $(postgresHEADERS) $(postgresUIH)
pkginclude_dir = $(includedir)/qgis


CLEANFILES = $(BUILT_SOURCES)

EXTRA_DIST = $(postgresHEADERS) $(postgresSOURCES) $(postgresUIC) $(qgis_UIC) $(qgis_UIH) $(postgresUIH) xpm/*.png xpm/*.xpm xpm/16x16/mapserver.xpm xpm/16x16/camera.png 
noinst_HEADERS = qgsdatasourceuri.h
