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

SUBDIRS = ui legend svg images themes resources raster composer $(MAC)

if !HAVE_QTMAC
PREFIX=-DPREFIX=\"$(prefix)\"
PLUGINPATH=-DPLUGINPATH=\"$(pkglibdir)\"
PKGDATAPATH=-DPKGDATAPATH=\"$(pkgdatadir)\"
endif
WIDGETPATH=../widgets
%.moc.cpp: %.h
	$(MOC) -o $@ $<

## main app
if HAVE_QTMAC
## Qt/Mac binary must be in MacOS directory within bundle, not in bin subdirectory
## (configure ensures that prefix ends in qgis.app/Contents/MacOS)
macbindir = $(exec_prefix)
macbin_PROGRAMS = qgis
else
bin_PROGRAMS = qgis
endif


## ancillary library for plug-ins
## Changed by Tim so lib builds first then app
lib_LTLIBRARIES = libqgis.la



qgis_SOURCES =	main.cpp			

AM_YFLAGS       = -d
qgis_YACC       = qgssearchstringparser.h

postgresHEADERS = qgsdbsourceselect.h		\
		  qgsnewconnection.h		\
		  qgspgquerybuilder.h

postgresSOURCES = qgsdbsourceselect.cpp		\
		  qgsnewconnection.cpp		\
		  qgspgquerybuilder.cpp

postgresUIH     = 
postgresLIBHEADERS = qgspgutil.h
postgresLIBSOURCES = qgspgutil.cpp

postgresUIHEADERS     = ui_qgsdbsourceselectbase.h	\
		  ui_qgsnewconnectionbase.h		\
		  ui_qgspgquerybuilderbase.h	
if HAVE_POSTGRESQL

postgresMOC       = $(postgresHEADERS:.h=.moc.cpp)

qgis_SOURCES     = $(postgresSOURCES)
endif


## nodist_qgis_SOURCES = $(qgis_MOC) $(qgis_UISOURCES) $(qgis_UIMOC) \
nodist_qgis_SOURCES = $(qgis_MOC) \
                      $(postgresMOC) $(postgresUISOURCES) $(postgresUIMOC)


## BUILT_SOURCES = $(qgis_MOC) $(qgis_UIMOC) $(qgis_UIHEADERS) $(qgis_UISOURCES) $(qgis_YACC) \
BUILT_SOURCES = $(qgis_MOC)  $(qgis_YACC) \
					$(postgresMOC) $(postgresUIMOC) 


qgis_LDADD = raster/libqgis_raster.la legend/libqgis_legend.la composer/libqgis_composer.la $(LDADD) $(QT_LDADD) $(GDAL_LDADD) $(PG_LIB) $(GEOS_LDADD) $(PYTHON_LIB) -lproj -lsqlite3 libqgis.la 
if HAVE_QTMAC
qgis_LDFLAGS = -framework ApplicationServices
else
qgis_LDFLAGS = -rdynamic
endif
qgis_CXXFLAGS = $(PREFIX) $(PLUGINPATH) $(PKGDATAPATH) $(GDAL_CFLAGS) $(CXXFLAGS) $(EXTRA_CXXFLAGS) $(QT_CXXFLAGS) $(PG_INC) $(DEBUG_QGIS)  $(HAVE_PYTHON) $(GEOS_CFLAGS) $(PYTHON_INCLUDE_DIR) -I./ui/ -I../widgets/projectionselector/ -I./legend/ -I./raster/ -I./composer/

libqgisHEADERS =							\
		qgisapp.h							\
		qgsacetatelines.h					\
		qgsacetateobject.h					\
		qgsacetaterectangle.h				\
		qgsattributeaction.h				\
		qgsclipper.h						\
		qgscolortable.h						\
		qgscontinuouscolrenderer.h			\
		qgscoordinatetransform.h			\
		qgscustomprojectiondialog.h			\
		qgsexception.h						\
		qgsfeature.h						\
		qgsfeatureattribute.h				\
		qgsfield.h							\
		qgsfillstylewidget.h				\
		qgsgeometry.h						\
		qgsgeometryvertexindex.h			\
		qgsgraduatedsymrenderer.h			\
		qgslabel.h							\
		qgslinestylewidget.h				\
		qgsmapcanvas.h						\
		qgsmapoverviewcanvas.h				\
		qgsmaplayer.h						\
		qgsmaplayerregistry.h				\
		qgsmaptopixel.h						\
		qgsmarkercatalogue.h				\
		qgsmarkersymbol.h					\
		qgsnumericsortlistviewitem.h			\
		qgspoint.h							\
		qgspointstylewidget.h				\
		qgsprojectproperty.h				\
		qgsprovidercountcalcevent.h			\
		qgsproviderextentcalcevent.h		\
		qgsproviderregistry.h				\
		qgsrangerenderitem.h				\
		qgsrect.h							\
		qgsrenderer.h						\
		qgsrenderitem.h						\
		qgsrunprocess.h						\
		qgsscalecalculator.h				\
		qgssinglesymrenderer.h				\
		qgsspatialrefsys.h					\
		qgssymbol.h							\
		qgsuniquevalrenderer.h				\
		qgsvectordataprovider.h				\
		qgsvectorfilewriter.h				\
		qgis.h						\
		qgisiface.h					\
		qgisinterface.h					\
		qgsabout.h				\
		qgsaddattrdialog.h				\
		qgsadvancedattrsearch.h				\
		qgsattributeactiondialog.h			\
		qgsattributedialog.h				\
		qgsattributetabledisplay.h			\
		qgsattributetable.h				\
		qgsbookmarks.h			\
		qgsbookmarkitem.h			\
		qgsclipboard.h					\
		qgscontexthelp.h				\
		qgscontcoldialog.h				\
		qgsencodingfiledialog.h				\
		qgsmaptopixel.h					\
		qgscustomsymbol.h				\
		qgsdatamanager.h				\
		qgsdataprovider.h				\
		qgsdatasource.h					\
		qgsdatasourceuri.h				\
		qgsdelattrdialog.h				\
		qgsdistancearea.h				\
		qgsvectorlayerproperties.h			\
		qgsexception.h					\
		qgsfile.h					\
		qgscsexception.h				\
		qgsgeomtypedialog.h				\
		qgsgrasydialog.h				\
		qgshelpviewer.h					\
		qgsidentifyresults.h				\
		qgslabelattributes.h				\
		qgslabeldialog.h				\
		qgsline.h					\
		qgslinestyledialog.h				\
		qgslinesymbol.h					\
		qgsludialog.h					\
		qgsmaplayerinterface.h				\
		qgsmapserverexport.h				\
		qgsmarkerdialog.h				\
		qgsmeasure.h					\
		qgsnewhttpconnection.h				\
		qgsoptions.h					\
		qgspastetransformations.h			\
		qgspatterndialog.h				\
		qgspluginitem.h					\
		qgspluginmanager.h				\
		qgspluginmetadata.h				\
		qgspluginregistry.h				\
		qgspolygonsymbol.h				\
		qgsproject.h					\
		qgsprojectproperties.h			\
		qgsprovidermetadata.h			\
		qgsrasterdataprovider.h			\
		qgsrasterlayerproperties.h		\
        qgssearchstring.h				\
        qgssearchtreenode.h				\
		qgsserversourceselect.h				\
		qgssisydialog.h					\
		qgssvgcache.h					\
		qgsmarkercatalogue.h				\
		qgssymbologyutils.h				\
		qgsuvaldialog.h					\
		qgslayerprojectionselector.h \
		$(WIDGETPATH)/projectionselector/qgsprojectionselector.h 


## all the ui files generate a corresponding header file
## libqgis_la_UIHEADERS = $(libqgis_la_UIC:.ui=.uic.h)
#libqgis_la_UIHEADERS = $(libqgis_la_UIC:.ui=ui_%.h)

if !NO_UIC_IMPLEMENTATIONS
## similarly all the ui files generate a corresponding implementation file
## libqgis_la_UISOURCES = $(libqgis_la_UIC:.ui=.uic.cpp)
endif

## these are optional ancillary user-defined qtdesigner-based headers
libqgis_la_UIH =							\
		qgisappbase.ui.h					\
		qgscustomprojectiondialogbase.ui.h

## these generated files need MOC files, too
## libqgis_la_UIMOC = $(libqgis_la_UIC:.ui=.moc.uic.cpp)

## non-uic files generated from MOC
libqgis_la_MOC =							\
		qgisapp.moc.cpp						\
		qgscoordinatetransform.moc.cpp		\
		qgscustomprojectiondialog.moc.cpp 	\
		qgsdataprovider.moc.cpp				\
		qgsfillstylewidget.moc.cpp			\
		qgslinestylewidget.moc.cpp			\
		qgsmapcanvas.moc.cpp				\
		qgsmaplayer.moc.cpp					\
		qgsmaplayerregistry.moc.cpp			\
		qgsmapoverviewcanvas.moc.cpp		\
		qgspointstylewidget.moc.cpp			\
		qgsrunprocess.moc.cpp				\
		qgsvectorlayer.moc.cpp \
		qgisinterface.moc.cpp			\
		qgsabout.moc.cpp		\
		qgsaddattrdialog.moc.cpp		\
		qgsadvancedattrsearch.moc.cpp		\
		qgsattributeactiondialog.moc.cpp	\
		qgsattributedialog.moc.cpp		\
		qgsattributetabledisplay.moc.cpp		\
		qgsattributetable.moc.cpp		\
		qgsbookmarks.moc.cpp		\
		qgscontcoldialog.moc.cpp		\
		qgscontexthelp.moc.cpp			\
		qgsdelattrdialog.moc.cpp		\
		qgsvectorlayerproperties.moc.cpp	\
		qgsencodingfiledialog.moc.cpp		\
		qgsgeomtypedialog.moc.cpp		\
		qgsgrasydialog.moc.cpp			\
		qgshelpviewer.moc.cpp			\
		qgsidentifyresults.moc.cpp		\
		qgslabeldialog.moc.cpp			\
		qgslayerprojectionselector.moc.cpp \
		qgslinestyledialog.moc.cpp		\
		qgsludialog.moc.cpp			\
		qgsmaplayerinterface.moc.cpp		\
		qgsmapserverexport.moc.cpp		\
		qgsmarkerdialog.moc.cpp			\
		qgsmeasure.moc.cpp			\
		qgsnewhttpconnection.moc.cpp		\
		qgsoptions.moc.cpp			\
		qgspastetransformations.moc.cpp		\
		qgspatterndialog.moc.cpp		\
		qgspluginmanager.moc.cpp		\
		qgsprojectproperties.moc.cpp		\
		qgsrasterdataprovider.moc.cpp		\
		qgsrasterlayerproperties.moc.cpp	\
		qgsserversourceselect.moc.cpp		\
		qgssisydialog.moc.cpp			\
		qgsuvaldialog.moc.cpp			\
		$(WIDGETPATH)/projectionselector/qgsprojectionselector.moc.cpp

libqgis_la_SOURCES =						\
		qgisapp.cpp							\
		qgsacetatelines.cpp					\
		qgsacetateobject.cpp				\
		qgsacetaterectangle.cpp				\
		qgsattributeaction.cpp				\
		qgsclipper.cpp						\
		qgscolortable.cpp					\
		qgscontinuouscolrenderer.cpp		\
		qgscoordinatetransform.cpp			\
		qgscustomprojectiondialog.cpp			\
		qgsdatamanager.cpp				\
		qgsexception.cpp				\
		qgsfeature.cpp					\
		qgsfeatureattribute.cpp				\
		qgsfield.cpp						\
		qgsfillstylewidget.cpp				\
		qgsgeometry.cpp						\
		qgsgeometryvertexindex.cpp			\
		qgsgraduatedsymrenderer.cpp			\
		qgslabel.cpp						\
		qgslinestylewidget.cpp				\
		qgsmapcanvas.cpp					\
		qgsmapoverviewcanvas.cpp			\
		qgsmaplayer.cpp						\
		qgsmaplayerregistry.cpp				\
		qgsmaptopixel.cpp					\
		qgsmarkercatalogue.cpp				\
		qgsmarkersymbol.cpp					\
		qgsnumericsortlistviewitem.cpp			\
		qgspoint.cpp						\
		qgspointstylewidget.cpp				\
		qgsprojectproperty.cpp				\
		qgsprovidercountcalcevent.cpp		\
		qgsproviderextentcalcevent.cpp		\
		qgsproviderregistry.cpp				\
		qgsrangerenderitem.cpp				\
		qgsrect.cpp							\
		qgsrenderer.cpp						\
		qgsrenderitem.cpp					\
		qgsrunprocess.cpp					\
		qgsscalecalculator.cpp				\
		qgssinglesymrenderer.cpp			\
		qgsspatialrefsys.cpp				\
		qgssymbol.cpp						\
		qgsuniquevalrenderer.cpp			\
		qgsvectordataprovider.cpp			\
		qgsvectorfilewriter.cpp				\
		qgsvectorlayer.cpp \
		qgisiface.cpp			\
		qgisinterface.cpp		\
		qgsabout.cpp		\
		qgsaddattrdialog.cpp		\
		qgsadvancedattrsearch.cpp	\
		qgsattributeactiondialog.cpp	\
		qgsattributedialog.cpp		\
		qgsattributetable.cpp		\
		qgsattributetabledisplay.cpp	\
		qgsbookmarks.cpp	\
		qgsbookmarkitem.cpp	\
		qgsclipboard.cpp		\
		qgscontcoldialog.cpp		\
		qgscontexthelp.cpp		\
		qgscustomsymbol.cpp		\
		qgsdatasource.cpp		\
		qgsdelattrdialog.cpp		\
		qgsdistancearea.cpp			\
		qgsvectorlayerproperties.cpp	\
		qgsencodingfiledialog.cpp	\
		qgsfile.cpp			\
		qgsgeomtypedialog.cpp		\
		qgsgrasydialog.cpp		\
		qgshelpviewer.cpp		\
		qgsidentifyresults.cpp		\
		qgslabelattributes.cpp		\
		qgslabeldialog.cpp		\
		qgslayerprojectionselector.cpp \
		qgsline.cpp			\
		qgslinestyledialog.cpp		\
		qgslinesymbol.cpp		\
		qgsludialog.cpp			\
		qgsmapserverexport.cpp		\
		qgsmarkerdialog.cpp		\
		qgsmeasure.cpp			\
		qgsnewhttpconnection.cpp	\
		qgsoptions.cpp			\
		qgspastetransformations.cpp	\
		qgspatterndialog.cpp		\
		qgspluginitem.cpp		\
		qgspluginmanager.cpp		\
		qgspluginmetadata.cpp		\
		qgspluginregistry.cpp		\
		qgspolygonsymbol.cpp		\
		qgsproject.cpp			\
		qgsprojectproperties.cpp	\
		qgsprovidermetadata.cpp		\
		qgsrasterdataprovider.cpp	\
		qgsrasterlayerproperties.cpp	\
        qgssearchstring.cpp             \
        qgssearchtreenode.cpp		\
        qgssearchstringlexer.ll		\
        qgssearchstringparser.yy	\
		qgsserversourceselect.cpp	\
		qgssisydialog.cpp		\
		qgssvgcache.cpp			\
		qgssymbologyutils.cpp		\
		qgsuvaldialog.cpp		\
		$(WIDGETPATH)/projectionselector/qgsprojectionselector.cpp

if HAVE_POSTGRESQL
libqgis_la_SOURCES += $(postgresLIBSOURCES)
endif


nodist_libqgis_la_SOURCES = $(libqgis_la_MOC) $(libqgis_la_UISOURCES) $(libqgis_la_UIMOC)

## BUILT_SOURCES += $(libqgis_la_MOC) $(libqgis_la_UIMOC) $(libqgis_la_UIHEADERS) $(libqgis_la_UISOURCES)
BUILT_SOURCES += $(libqgis_la_MOC) $(libqgis_la_UIHEADERS) 


libqgis_la_LIBADD = raster/libqgis_raster.la legend/libqgis_legend.la composer/libqgis_composer.la $(QT_LDADD) $(GEOS_LDADD) $(GDAL_LDADD) $(PYTHON_LIB) -lsqlite3
libqgis_la_LDFLAGS = -version-info $(INTERFACE_VERSION)

libqgis_la_CXXFLAGS = $(PREFIX) $(PLUGINPATH) $(PKGDATAPATH) $(GDAL_CFLAGS) $(CXXFLAGS) $(EXTRA_CXXFLAGS) $(QT_CXXFLAGS) $(PG_INC) $(DEBUG_QGIS) $(GEOS_CFLAGS) $(PYTHON_INCLUDE_DIR) $(HAVE_PYTHON) -I./ui/ -I../widgets/projectionselector/ -I./legend/ -I./raster/ -I./composer/

## for installing headers in $(includedir)/qgis
pkginclude_HEADERS = $(qgisHEADERS) \
		$(qgis_UIH) \
		$(postgresHEADERS) \
		$(postgresUIH) \
		$(postgresLIBHEADERS) \
		$(libqgisHEADERS) \
		$(libqgis_la_UIH)

nodist_pkginclude_HEADERS = $(qgis_UIHEADERS) \
		$(libqgis_la_UIHEADERS)

pkginclude_dir = $(includedir)/qgis


CLEANFILES = $(BUILT_SOURCES)

EXTRA_DIST = $(postgresSOURCES) \
		$(postgresUIC) \
		$(postgresLIBSOURCES) \
		$(qgis_UIC) \
		$(libqgis_la_UIC) \
		xpm/*.png \
		xpm/*.xpm \
		xpm/16x16/mapserver.xpm \
		xpm/16x16/camera.png 

noinst_HEADERS = qgsdatasourceuri.h
