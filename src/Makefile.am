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

if !HAVE_QTMAC
PREFIX=-DPREFIX=\"$(prefix)\"
PLUGINPATH=-DPLUGINPATH=\"$(pkglibdir)\"
PKGDATAPATH=-DPKGDATAPATH=\"$(pkgdatadir)\"
endif
WIDGETPATH=../widgets
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
		qgsaddattrdialog.h				\
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
		qgscomposerscalebar.h				\
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
		qgsspatialrefsys.h	\
		qgsspatialreferences.h	\
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
		splashscreen.h				\
		qgslayerprojectionselector.h \
		$(WIDGETPATH)/projectionselector/qgsprojectionselector.h \
		qgscustomprojectiondialog.h \
		qgspointstylewidget.h				\
		qgslinestylewidget.h				\
		qgsfillstylewidget.h				\
		splashscreen.h				

#		qgsvectorsymbologywidget.h			

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
		qgscomposerscalebarbase.ui		\
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
		qgslayerprojectionselectorbase.ui \
		$(WIDGETPATH)/projectionselector/qgsprojectionselectorbase.ui \
		qgscustomprojectiondialogbase.ui \
		qgsuvalmadialogbase.ui			\
		qgspointstylewidgetbase.ui		\
		qgsfillstylewidgetbase.ui		\
		qgslinestylewidgetbase.ui

#		qgsvectorsymbologywidgetbase.ui		

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
		qgslayerprojectionselectorbase.ui.h \
		qgsprojectpropertiesbase.ui.h	\
		qgscustomprojectiondialogbase.ui.h

## these generated files need MOC files, too
qgis_UIMOC = $(qgis_UIC:.ui=.moc.uic.cpp)


## non-uic files generated from MOC

qgis_MOC =	qgisapp.moc.cpp				\
		qgisinterface.moc.cpp			\
		qgsaddattrdialog.moc.cpp		\
		qgsattributeactiondialog.moc.cpp	\
		qgsattributedialog.moc.cpp		\
		qgsattributetabledisplay.moc.cpp		\
		qgsattributetable.moc.cpp		\
		qgscomposerlabel.moc.cpp		\
		qgscomposermap.moc.cpp			\
		qgscomposer.moc.cpp			\
		qgscomposerscalebar.moc.cpp		\
		qgscomposervectorlegend.moc.cpp		\
		qgscomposerview.moc.cpp			\
		qgscomposition.moc.cpp			\
		qgscontcoldialog.moc.cpp		\
		qgscoordinatetransform.moc.cpp		\
		qgsdataprovider.moc.cpp		\
		qgsdelattrdialog.moc.cpp			\
		qgsdlgvectorlayerproperties.moc.cpp	\
		qgsencodingfiledialog.moc.cpp		\
		qgsfillstylewidget.moc.cpp		\
		qgsgeomtypedialog.moc.cpp		\
		qgsgramadialog.moc.cpp			\
		qgsgramaextensionwidget.moc.cpp		\
		qgsgrasydialog.moc.cpp			\
		qgshelpviewer.moc.cpp			\
		qgsidentifyresults.moc.cpp		\
		qgslabeldialog.moc.cpp			\
		qgslayerprojectionselector.moc.cpp \
		qgslegend.moc.cpp			\
		qgslinestyledialog.moc.cpp		\
		qgslinestylewidget.moc.cpp		\
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
		qgspointstylewidget.moc.cpp 		\
		qgsprojectproperties.moc.cpp		\
		qgsrasterlayer.moc.cpp			\
		qgsrasterlayerproperties.moc.cpp	\
		qgsrunprocess.moc.cpp			\
		qgssimadialog.moc.cpp			\
		qgssisydialog.moc.cpp			\
		qgsuvaldialog.moc.cpp			\
		qgsuvalmadialog.moc.cpp			\
		qgsvectordataprovider.moc.cpp		\
		qgsvectorlayer.moc.cpp			\
		splashscreen.moc.cpp		\
		$(WIDGETPATH)/projectionselector/qgsprojectionselector.moc.cpp \
		qgscustomprojectiondialog.moc.cpp

#		qgsvectorsymbologywidget.moc.cpp	

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
		qgscomposeritem.cpp		\
		qgscomposerlabel.cpp		\
		qgscomposermap.cpp		\
		qgscomposerscalebar.cpp		\
		qgscomposervectorlegend.cpp	\
		qgscomposerview.cpp		\
		qgscomposition.cpp		\
		qgscontcoldialog.cpp		\
		qgscontinuouscolrenderer.cpp	\
		qgscoordinatetransform.cpp	\
		qgscustomsymbol.cpp		\
		qgsdatasource.cpp		\
		qgsdelattrdialog.cpp		\
		qgsdlgvectorlayerproperties.cpp	\
		qgsencodingfiledialog.cpp	\
		qgsfeatureattribute.cpp		\
		qgsfeature.cpp			\
		qgsfield.cpp			\
		qgsfillstylewidget.cpp		\
		qgsgeomtypedialog.cpp		\
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
		qgslayerprojectionselector.cpp \
		qgslegend.cpp		\
		qgslegenditem.cpp		\
		qgsline.cpp			\
		qgslinestyledialog.cpp		\
		qgslinestylewidget.cpp		\
		qgslinesymbol.cpp		\
		qgsludialog.cpp			\
		qgsmapcanvas.cpp		\
		qgsmaplayer.cpp			\
		qgsmaplayerregistry.cpp		\
		qgsmapserverexport.cpp		\
		qgsmaptopixel.cpp		\
		qgsmarkercatalogue.cpp		\
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
		qgspointstylewidget.cpp		\
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
		qgsrenderer.cpp 		\
		qgsrenderitem.cpp		\
		qgsrunprocess.cpp		\
		qgsscalecalculator.cpp		\
		qgssimadialog.cpp		\
		qgssimarenderer.cpp		\
		qgssinglesymrenderer.cpp	\
		qgssisydialog.cpp		\
		qgsspatialreferences.cpp	\
		qgsspatialrefsys.cpp	\
		qgssvgcache.cpp			\
		qgssymbol.cpp			\
		qgssymbologyutils.cpp		\
		qgsuniquevalrenderer.cpp	\
		qgsuvaldialog.cpp		\
		qgsuvalmadialog.cpp		\
		qgsuvalmarenderer.cpp		\
		qgsvectordataprovider.cpp	\
		qgsvectorfilewriter.cpp		\
		qgsvectorlayer.cpp		\
		splashscreen.cpp \
		$(WIDGETPATH)/projectionselector/qgsprojectionselector.cpp \
		qgscustomprojectiondialog.cpp

#		qgsvectorsymbologywidget.cpp	

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


qgis_LDADD = $(LDADD) $(QT_LDADD) $(GDAL_LDADD) $(PG_LIB) $(GEOS_LDADD) -lproj -lsqlite3 libqgis.la
if HAVE_QTMAC
qgis_LDFLAGS = -framework ApplicationServices
else
qgis_LDFLAGS = -rdynamic
endif
qgis_CXXFLAGS = $(PREFIX) $(PLUGINPATH) $(PKGDATAPATH) $(GDAL_CFLAGS) $(CXXFLAGS) $(EXTRA_CXXFLAGS) $(QT_CXXFLAGS) -I$(PG_INC) $(DEBUG_QGIS) $(GEOS_CFLAGS) -I../widgets/projectionselector/

libqgis_la_SOURCES =				\
		qgisappbase.moc.uic.cpp		\
		qgisappbase.uic.cpp		\
		qgisapp.cpp			\
		qgisapp.moc.cpp			\
		qgsacetateobject.cpp		\
		qgsacetaterectangle.cpp		\
		qgsattributeaction.cpp		\
		qgsclipper.cpp			\
		qgscolortable.cpp		\
		qgscomposerbase.moc.uic.cpp	\
		qgscomposerbase.uic.cpp		\
		qgscomposer.cpp			\
		qgscomposeritem.cpp		\
		qgscomposerlabelbase.moc.uic.cpp	\
		qgscomposerlabelbase.uic.cpp		\
		qgscomposerlabel.cpp		\
		qgscomposerlabel.moc.cpp \
		qgscomposermapbase.moc.uic.cpp  \
		qgscomposermapbase.uic.cpp		\
		qgscomposermap.cpp		\
		qgscomposermap.moc.cpp		\
		qgscomposer.moc.cpp		\
		qgscomposerscalebarbase.moc.uic.cpp	\
		qgscomposerscalebarbase.uic.cpp	\
		qgscomposerscalebar.cpp		\
		qgscomposerscalebar.moc.cpp	\
		qgscomposervectorlegendbase.moc.uic.cpp \
		qgscomposervectorlegendbase.uic.cpp \
		qgscomposervectorlegend.cpp	\
		qgscomposervectorlegend.moc.cpp \
		qgscomposerview.cpp		\
		qgscomposerview.moc.cpp		\
		qgscompositionbase.moc.uic.cpp	\
		qgscompositionbase.uic.cpp	\
		qgscomposition.cpp		\
		qgscomposition.moc.cpp		\
		qgscontinuouscolrenderer.cpp	\
		qgscoordinatetransform.cpp	\
		qgscoordinatetransform.moc.cpp  \
		qgsdataprovider.moc.cpp         \
		qgsfeatureattribute.cpp		\
		qgsfeature.cpp			\
		qgsfield.cpp			\
		qgsfillstylewidgetbase.moc.uic.cpp	\
		qgsfillstylewidgetbase.uic.cpp		\
		qgsfillstylewidget.cpp		\
		qgsfillstylewidget.moc.cpp	\
		qgsgraduatedmarenderer.cpp	\
		qgsgraduatedsymrenderer.cpp	\
		qgslabel.cpp			\
		qgslegend.cpp			\
		qgslegenditem.cpp		\
		qgslegenditem.moc.cpp			\
		qgslegend.moc.cpp			\
		qgslinestylewidgetbase.moc.uic.cpp	\
		qgslinestylewidgetbase.uic.cpp		\
		qgslinestylewidget.cpp		\
		qgslinestylewidget.moc.cpp	\
		qgsmapcanvas.cpp		\
		qgsmapcanvas.moc.cpp		\
		qgsmaplayer.cpp			\
		qgsmaplayer.moc.cpp		\
		qgsmaplayerregistry.cpp		\
		qgsmaplayerregistry.moc.cpp	\
		qgsmaptopixel.cpp		\
		qgsmarkercatalogue.cpp		\
		qgsmarkersymbol.cpp		\
		qgspoint.cpp			\
		qgspoint.moc.cpp		\
		qgspointstylewidgetbase.moc.uic.cpp	\
		qgspointstylewidgetbase.uic.cpp	\
		qgspointstylewidget.cpp		\
		qgspointstylewidget.moc.cpp	\
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
		qgsspatialreferences.cpp  \
		qgsspatialrefsys.cpp \
		qgssymbol.cpp			\
		qgsuniquevalrenderer.cpp	\
		qgsuvalmarenderer.cpp		\
		qgsvectordataprovider.cpp	\
		qgsvectordataprovider.moc.cpp	\
		qgsvectorfilewriter.cpp		\
		qgsvectorlayer.cpp		\
		qgsvectorlayer.moc.cpp \
		qgscustomprojectiondialog.moc.cpp 

if HAVE_POSTGRESQL
libqgis_la_SOURCES += $(postgresLIBSOURCES)
endif

libqgis_la_LIBFLAGS = $(QT_LDADD) $(GEOS_LDADD) $(GDAL_LDADD)
libqgis_la_LDFLAGS = -version-info $(INTERFACE_VERSION) $(QT_LDADD) $(GDAL_LDADD) -lsqlite3

libqgis_la_CXXFLAGS = $(PREFIX) $(PLUGINPATH) $(PKGDATAPATH) $(GDAL_CFLAGS) $(CXXFLAGS) $(EXTRA_CXXFLAGS) $(QT_CXXFLAGS) -I$(PG_INC) $(DEBUG_QGIS) $(GEOS_CFLAGS) -I../widgets/projectionselector/

## for installing headers in $(includedir)/qgis
pkginclude_HEADERS = $(headers) $(qgis_UIHEADERS) $(qgis_UIH) $(postgresHEADERS) $(postgresUIH)
pkginclude_dir = $(includedir)/qgis


CLEANFILES = $(BUILT_SOURCES)

EXTRA_DIST = $(postgresHEADERS) $(postgresSOURCES) $(postgresUIC) $(qgis_UIC) $(qgis_UIH) $(postgresUIH) xpm/*.png xpm/*.xpm xpm/16x16/mapserver.xpm xpm/16x16/camera.png 
noinst_HEADERS = qgsdatasourceuri.h
