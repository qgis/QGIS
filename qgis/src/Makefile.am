# Copyright (C) 2003 Gary Sherman <sherman at mrcc.com>
#  
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without 
# modifications, as long as this notice is preserved.
# 
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

if HAVE_QTMAC
MAC = mac
endif

SUBDIRS = legend svg images themes resources $(MAC)

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

if HAVE_QT3
%.uic.h: %.ui 
	$(UIC) -o $@ $<
endif

if HAVE_QT4
%.uic.h: %.ui 
	$(UIC) -o $@ $<
	../tools/qgis-qt3to4.pl -uic $@
endif

if !NO_UIC_IMPLEMENTATIONS
%.uic.cpp: %.ui
	$(UIC) -o $@ -impl $*.uic.h $<
endif

## main app
if HAVE_QTMAC
# Qt/Mac binary must be in MacOS directory within bundle, not in bin subdirectory
# (configure ensures that prefix ends in qgis.app/Contents/MacOS)
macbindir = $(exec_prefix)
macbin_PROGRAMS = qgis
else
bin_PROGRAMS = qgis
endif

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
		qgsadvancedattrsearch.h				\
		qgsattributeaction.h				\
		qgsattributeactiondialog.h			\
		qgsattributedialog.h				\
		qgsattributetabledisplay.h			\
		qgsattributetable.h				\
		qgsbookmarks.h			\
		qgsbookmarkitem.h			\
		qgsclipboard.h					\
		qgsclipper.h					\
		qgscolortable.h					\
		qgscomposer.h					\
		qgscomposition.h				\
		qgscomposeritem.h				\
		qgscomposermap.h				\
		qgscomposervectorlegend.h			\
		qgscomposerlabel.h				\
		qgscomposerpicture.h				\
		qgscomposerscalebar.h				\
		qgscomposerview.h				\
		qgscomposition.h				\
		qgscontexthelp.h				\
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
		qgsgeometry.h					\
		qgsgeometryvertexindex.h			\
		qgsgeomtypedialog.h				\
		qgsgraduatedsymrenderer.h			\
		qgsgrasydialog.h				\
		qgshelpviewer.h					\
		qgsidentifyresults.h				\
		qgslabelattributes.h				\
		qgslabeldialog.h				\
		qgslabel.h					\
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
		qgsnewhttpconnection.h				\
		qgsoptions.h					\
		qgspastetransformations.h			\
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
		qgsrasterdataprovider.h				\
		qgsrasterlayer.h				\
		qgsrasterlayerproperties.h			\
		qgsrect.h					\
		qgsrenderer.h					\
		qgsrenderitem.h					\
		qgsrunprocess.h					\
		qgsscalecalculator.h				\
                qgssearchstring.h				\
                qgssearchtreenode.h				\
		qgsserversourceselect.h				\
		qgssinglesymrenderer.h				\
		qgssisydialog.h					\
		qgsspatialrefsys.h	\
		qgssvgcache.h					\
		qgsmarkercatalogue.h				\
		qgssymbol.h					\
		qgssymbologyutils.h				\
		qgsuniquevalrenderer.h				\
		qgsuvaldialog.h					\
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


## qt designer files for all the GUI stuff

qgis_UIC =	qgisappbase.ui				\
		qgsabout.ui				\
		qgsaddattrdialogbase.ui			\
		qgsadvancedattrsearchbase.ui		\
		qgsattributedialogbase.ui		\
		qgsattributeactiondialogbase.ui		\
		qgsattributetablebase.ui		\
		qgsbookmarksbase.ui		\
		qgscomposerbase.ui			\
		qgscompositionbase.ui			\
		qgscomposermapbase.ui			\
		qgscomposervectorlegendbase.ui		\
		qgscomposerlabelbase.ui		     	\
		qgscomposerpicturebase.ui	     	\
		qgscomposerscalebarbase.ui		\
		qgscontcoldialogbase.ui			\
		qgsdelattrdialogbase.ui			\
		qgsdlgvectorlayerpropertiesbase.ui	\
		qgsgeomtypedialogbase.ui		\
		qgsgrasydialogbase.ui			\
		qgshelpviewerbase.ui			\
		qgsidentifyresultsbase.ui		\
		qgslabeldialogbase.ui			\
		qgslinestyledialogbase.ui		\
		qgsludialogbase.ui			\
		qgsmapserverexportbase.ui		\
		qgsmarkerdialogbase.ui			\
		qgsmessageviewer.ui			\
		qgsmeasurebase.ui			\
		qgsnewhttpconnectionbase.ui		\
		qgsoptionsbase.ui			\
		qgspastetransformationsbase.ui		\
		qgspatterndialogbase.ui			\
		qgspluginmanagerbase.ui			\
		qgsprojectpropertiesbase.ui		\
		qgsrasterlayerpropertiesbase.ui		\
		qgsserversourceselectbase.ui		\
		qgssisydialogbase.ui			\
		qgsuvaldialogbase.ui			\
		qgslayerprojectionselectorbase.ui \
		$(WIDGETPATH)/projectionselector/qgsprojectionselectorbase.ui \
		qgscustomprojectiondialogbase.ui \
		qgspointstylewidgetbase.ui		\
		qgsfillstylewidgetbase.ui		\
		qgslinestylewidgetbase.ui

## all the ui files generate a corresponding header file
qgis_UIHEADERS = $(qgis_UIC:.ui=.uic.h)

if !NO_UIC_IMPLEMENTATIONS
## similarly all the ui files generate a corresponding implementation file
qgis_UISOURCES = $(qgis_UIC:.ui=.uic.cpp)
endif

## these are optional ancillary user-defined qtdesigner-based headers
qgis_UIH =	qgisappbase.ui.h			\
		qgsabout.ui.h				\
		qgsattributetablebase.ui.h		\
		qgshelpviewerbase.ui.h			\
		qgsmapserverexportbase.ui.h		\
		qgsmessageviewer.ui.h			\
		qgsoptionsbase.ui.h			\
		qgsrasterlayerpropertiesbase.ui.h	\
		qgsserversourceselectbase.ui.h		\
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
		qgsadvancedattrsearch.moc.cpp		\
		qgsattributeactiondialog.moc.cpp	\
		qgsattributedialog.moc.cpp		\
		qgsattributetabledisplay.moc.cpp		\
		qgsattributetable.moc.cpp		\
		qgsbookmarks.moc.cpp		\
		qgscomposerlabel.moc.cpp		\
		qgscomposerpicture.moc.cpp		\
		qgscomposermap.moc.cpp			\
		qgscomposer.moc.cpp			\
		qgscomposerscalebar.moc.cpp		\
		qgscomposervectorlegend.moc.cpp		\
		qgscomposerview.moc.cpp			\
		qgscomposition.moc.cpp			\
		qgscontcoldialog.moc.cpp		\
		qgscontexthelp.moc.cpp			\
		qgscoordinatetransform.moc.cpp		\
		qgsdataprovider.moc.cpp		\
		qgsdelattrdialog.moc.cpp			\
		qgsdlgvectorlayerproperties.moc.cpp	\
		qgsencodingfiledialog.moc.cpp		\
		qgsfillstylewidget.moc.cpp		\
		qgsgeomtypedialog.moc.cpp		\
		qgsgrasydialog.moc.cpp			\
		qgshelpviewer.moc.cpp			\
		qgsidentifyresults.moc.cpp		\
		qgslabeldialog.moc.cpp			\
		qgslayerprojectionselector.moc.cpp \
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
		qgsnewhttpconnection.moc.cpp		\
		qgsoptions.moc.cpp			\
		qgspastetransformations.moc.cpp		\
		qgspatterndialog.moc.cpp		\
		qgspluginmanager.moc.cpp		\
		qgspointstylewidget.moc.cpp 		\
		qgsprojectproperties.moc.cpp		\
		qgsrasterdataprovider.moc.cpp		\
		qgsrasterlayer.moc.cpp			\
		qgsrasterlayerproperties.moc.cpp	\
		qgsrunprocess.moc.cpp			\
		qgsserversourceselect.moc.cpp		\
		qgssisydialog.moc.cpp			\
		qgsuvaldialog.moc.cpp			\
		qgsvectordataprovider.moc.cpp		\
		qgsvectorlayer.moc.cpp			\
		splashscreen.moc.cpp		\
		$(WIDGETPATH)/projectionselector/qgsprojectionselector.moc.cpp \
		qgscustomprojectiondialog.moc.cpp

qgis_SOURCES =	main.cpp			\
		qgisapp.cpp			\
		qgisiface.cpp			\
		qgisinterface.cpp		\
		qgsacetatelines.cpp		\
		qgsacetateobject.cpp		\
		qgsacetaterectangle.cpp		\
		qgsaddattrdialog.cpp		\
		qgsadvancedattrsearch.cpp	\
		qgsattributeaction.cpp		\
		qgsattributeactiondialog.cpp	\
		qgsattributedialog.cpp		\
		qgsattributetable.cpp		\
		qgsattributetabledisplay.cpp	\
		qgsbookmarks.cpp	\
		qgsbookmarkitem.cpp	\
		qgsclipboard.cpp		\
		qgsclipper.cpp			\
		qgscolortable.cpp		\
		qgscomposer.cpp			\
		qgscomposeritem.cpp		\
		qgscomposerlabel.cpp		\
		qgscomposerpicture.cpp		\
		qgscomposermap.cpp		\
		qgscomposerscalebar.cpp		\
		qgscomposervectorlegend.cpp	\
		qgscomposerview.cpp		\
		qgscomposition.cpp		\
		qgscontcoldialog.cpp		\
		qgscontexthelp.cpp		\
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
		qgsgeometry.cpp			\
		qgsgeometryvertexindex.cpp	\
		qgsgeomtypedialog.cpp		\
		qgsgraduatedsymrenderer.cpp	\
		qgsgrasydialog.cpp		\
		qgshelpviewer.cpp		\
		qgsidentifyresults.cpp		\
		qgslabelattributes.cpp		\
		qgslabel.cpp			\
		qgslabeldialog.cpp		\
		qgslayerprojectionselector.cpp \
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
		qgsnewhttpconnection.cpp	\
		qgsoptions.cpp			\
		qgspastetransformations.cpp	\
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
		qgsrasterdataprovider.cpp	\
		qgsrasterlayer.cpp		\
		qgsrasterlayerproperties.cpp	\
		qgsrect.cpp			\
		qgsrenderer.cpp 		\
		qgsrenderitem.cpp		\
		qgsrunprocess.cpp		\
		qgsscalecalculator.cpp		\
                qgssearchstring.cpp             \
                qgssearchtreenode.cpp		\
                qgssearchstringlexer.ll		\
                qgssearchstringparser.yy	\
		qgsserversourceselect.cpp	\
		qgssinglesymrenderer.cpp	\
		qgssisydialog.cpp		\
		qgsspatialrefsys.cpp	\
		qgssvgcache.cpp			\
		qgssymbol.cpp			\
		qgssymbologyutils.cpp		\
		qgsuniquevalrenderer.cpp	\
		qgsuvaldialog.cpp		\
		qgsvectordataprovider.cpp	\
		qgsvectorfilewriter.cpp		\
		qgsvectorlayer.cpp		\
		splashscreen.cpp \
		$(WIDGETPATH)/projectionselector/qgsprojectionselector.cpp \
		qgscustomprojectiondialog.cpp

AM_YFLAGS       = -d
qgis_YACC       = qgssearchstringparser.h

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
if !NO_UIC_IMPLEMENTATIONS
postgresUISOURCES = $(postgresUIC:.ui=.uic.cpp)
endif
postgresUIMOC     = $(postgresUIC:.ui=.moc.uic.cpp)

postgresMOC       = $(postgresHEADERS:.h=.moc.cpp)

qgis_SOURCES     += $(postgresSOURCES)
endif


nodist_qgis_SOURCES = $(qgis_MOC) $(qgis_UISOURCES) $(qgis_UIMOC) \
                      $(postgresMOC) $(postgresUISOURCES) $(postgresUIMOC)


BUILT_SOURCES = $(qgis_MOC) $(qgis_UIMOC) $(qgis_UIHEADERS) $(qgis_UISOURCES) $(qgis_YACC) \
					$(postgresMOC) $(postgresUIMOC) $(postgresUIHEADERS) $(postgresUISOURCES)


qgis_LDADD = $(LDADD) $(QT_LDADD) $(GDAL_LDADD) $(PG_LIB) $(GEOS_LDADD) -lproj -lsqlite3 libqgis.la legend/libqgis_legend.la
if HAVE_QTMAC
qgis_LDFLAGS = -framework ApplicationServices
else
qgis_LDFLAGS = -rdynamic
endif
qgis_CXXFLAGS = $(PREFIX) $(PLUGINPATH) $(PKGDATAPATH) $(GDAL_CFLAGS) $(CXXFLAGS) $(EXTRA_CXXFLAGS) $(QT_CXXFLAGS) -I$(PG_INC) $(DEBUG_QGIS) $(GEOS_CFLAGS) -I../widgets/projectionselector/ -I./legend/

libqgis_la_SOURCES =						\
		qgisapp.cpp					\
		qgisapp.moc.cpp					\
		qgisappbase.moc.uic.cpp				\
		qgsacetateobject.cpp				\
		qgsacetaterectangle.cpp				\
		qgsattributeaction.cpp				\
		qgsclipper.cpp					\
		qgscolortable.cpp				\
		qgscomposer.cpp					\
		qgscomposer.moc.cpp				\
		qgscomposerbase.moc.uic.cpp			\
		qgscomposeritem.cpp				\
		qgscomposerlabel.cpp				\
		qgscomposerlabel.moc.cpp			\
		qgscomposerlabelbase.moc.uic.cpp		\
		qgscomposerpicture.cpp				\
		qgscomposerpicture.moc.cpp			\
		qgscomposerpicturebase.moc.uic.cpp		\
		qgscomposermap.cpp				\
		qgscomposermap.moc.cpp				\
		qgscomposermapbase.moc.uic.cpp			\
		qgscomposerscalebar.cpp				\
		qgscomposerscalebar.moc.cpp			\
		qgscomposerscalebarbase.moc.uic.cpp		\
		qgscomposervectorlegend.cpp			\
		qgscomposervectorlegend.moc.cpp			\
		qgscomposervectorlegendbase.moc.uic.cpp		\
		qgscomposerview.cpp				\
		qgscomposerview.moc.cpp				\
		qgscomposition.cpp				\
		qgscomposition.moc.cpp				\
		qgscompositionbase.moc.uic.cpp			\
		qgscontinuouscolrenderer.cpp			\
		qgscoordinatetransform.cpp			\
		qgscoordinatetransform.moc.cpp			\
		qgscustomprojectiondialog.cpp			\
		qgscustomprojectiondialogbase.moc.uic.cpp	\
		qgscustomprojectiondialog.moc.cpp 		\
		qgsdataprovider.moc.cpp				\
		qgsexception.cpp				\
		qgsfeature.cpp					\
		qgsfeatureattribute.cpp				\
		qgsfield.cpp					\
		qgsfillstylewidget.cpp				\
		qgsfillstylewidget.moc.cpp			\
		qgsfillstylewidgetbase.moc.uic.cpp		\
		qgsgeometry.cpp					\
		qgsgeometryvertexindex.cpp			\
		qgsgraduatedsymrenderer.cpp			\
		qgslabel.cpp					\
		qgslinestylewidget.cpp				\
		qgslinestylewidget.moc.cpp			\
		qgslinestylewidgetbase.moc.uic.cpp		\
		qgsmapcanvas.cpp				\
		qgsmapcanvas.moc.cpp				\
		qgsmaplayer.cpp					\
		qgsmaplayer.moc.cpp				\
		qgsmaplayerregistry.cpp				\
		qgsmaplayerregistry.moc.cpp			\
		qgsmaptopixel.cpp				\
		qgsmarkercatalogue.cpp				\
		qgsmarkersymbol.cpp				\
		qgspoint.cpp					\
		qgspoint.moc.cpp				\
		qgspointstylewidget.cpp				\
		qgspointstylewidget.moc.cpp			\
		qgspointstylewidgetbase.moc.uic.cpp		\
		qgsprojectproperty.cpp				\
		qgsprovidercountcalcevent.cpp			\
		qgsproviderextentcalcevent.cpp			\
		qgsproviderregistry.cpp				\
		qgsrangerenderitem.cpp				\
		qgsrasterlayer.cpp				\
		qgsrasterlayer.moc.cpp				\
		qgsrect.cpp					\
		qgsrenderer.cpp					\
		qgsrenderitem.cpp				\
		qgsrunprocess.cpp				\
		qgsrunprocess.moc.cpp				\
		qgsscalecalculator.cpp				\
		qgssinglesymrenderer.cpp			\
		qgsspatialrefsys.cpp				\
		qgssymbol.cpp					\
		qgsuniquevalrenderer.cpp			\
		qgsvectordataprovider.cpp			\
		qgsvectordataprovider.moc.cpp			\
		qgsvectorfilewriter.cpp				\
		qgsvectorlayer.cpp				\
		qgsvectorlayer.moc.cpp

if !NO_UIC_IMPLEMENTATIONS
libqgis_la_SOURCES +=						\
		qgisappbase.uic.cpp				\
		qgscomposerbase.uic.cpp				\
		qgscomposerlabelbase.uic.cpp			\
		qgscomposerpicturebase.uic.cpp			\
		qgscomposermapbase.uic.cpp			\
		qgscomposerscalebarbase.uic.cpp			\
		qgscomposervectorlegendbase.uic.cpp		\
		qgscompositionbase.uic.cpp			\
		qgscustomprojectiondialogbase.uic.cpp		\
		qgsfillstylewidgetbase.uic.cpp			\
		qgslinestylewidgetbase.uic.cpp			\
		qgspointstylewidgetbase.uic.cpp
endif

if HAVE_POSTGRESQL
libqgis_la_SOURCES += $(postgresLIBSOURCES)
endif


libqgis_la_LIBFLAGS = $(QT_LDADD) $(GEOS_LDADD) $(GDAL_LDADD)
libqgis_la_LDFLAGS = -version-info $(INTERFACE_VERSION) $(QT_LDADD) $(GDAL_LDADD) -lsqlite3

libqgis_la_CXXFLAGS = $(PREFIX) $(PLUGINPATH) $(PKGDATAPATH) $(GDAL_CFLAGS) $(CXXFLAGS) $(EXTRA_CXXFLAGS) $(QT_CXXFLAGS) -I$(PG_INC) $(DEBUG_QGIS) $(GEOS_CFLAGS) -I../widgets/projectionselector/ -I./legend/

## for installing headers in $(includedir)/qgis
pkginclude_HEADERS = $(headers) \
		$(qgis_UIHEADERS) \
		$(qgis_UIH) \
		$(postgresHEADERS) \
		$(postgresUIH)

pkginclude_dir = $(includedir)/qgis


CLEANFILES = $(BUILT_SOURCES)

EXTRA_DIST = $(postgresHEADERS) \
		$(postgresSOURCES) \
		$(postgresUIC) \
		$(qgis_UIC) \
		xpm/*.png \
		xpm/*.xpm \
		xpm/16x16/mapserver.xpm \
		xpm/16x16/camera.png 
		
noinst_HEADERS = qgsdatasourceuri.h
