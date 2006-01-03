# Copyright (C) 2003 Gary Sherman <sherman at mrcc.com>
#  
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without 
# modifications, as long as this notice is preserved.
# 
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

##
## ----------------------------------------------------------------------
##        Initial setup 
## ----------------------------------------------------------------------
##
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

##
## ----------------------------------------------------------------------
##      Define Targets
## ----------------------------------------------------------------------
##
if HAVE_QTMAC
## Qt/Mac binary must be in MacOS directory within bundle, not in bin subdirectory
## (configure ensures that prefix ends in qgis.app/Contents/MacOS)
macbindir = $(exec_prefix)
macbin_PROGRAMS = qgis
else
bin_PROGRAMS = qgis
endif

lib_LTLIBRARIES = libqgis.la


##
## ----------------------------------------------------------------------
##         QGIS Desktop Application
## ----------------------------------------------------------------------
##

qgis_SOURCES =	main.cpp

qgis_LDADD = raster/libqgis_raster.la legend/libqgis_legend.la composer/libqgis_composer.la $(LDADD) $(QT_LDADD) $(GDAL_LDADD) $(PG_LIB) $(GEOS_LDADD) $(PYTHON_LIB) -lproj -lsqlite3 libqgis.la 
if HAVE_QTMAC
qgis_LDFLAGS = -framework ApplicationServices
else
qgis_LDFLAGS = -rdynamic
endif
qgis_CXXFLAGS = $(PREFIX) $(PLUGINPATH) $(PKGDATAPATH) $(GDAL_CFLAGS) $(CXXFLAGS) $(EXTRA_CXXFLAGS) $(QT_CXXFLAGS) $(PG_INC) $(DEBUG_QGIS)  $(HAVE_PYTHON) $(GEOS_CFLAGS) $(PYTHON_INCLUDE_DIR) -I./ui/ -I../widgets/projectionselector/ -I./legend/ -I./raster/ -I./composer/

##
## ----------------------------------------------------------------------
##         libqgis stuff starts here
## ----------------------------------------------------------------------
##

libqgisHEADERS =							\
		qgisapp.h							\
		qgis.h						\
		qgisiface.h					\
		qgisinterface.h					\
		qgsabout.h				\
		qgsacetatelines.h					\
		qgsacetateobject.h					\
		qgsacetaterectangle.h				\
		qgsaddattrdialog.h				\
		qgsadvancedattrsearch.h				\
		qgsattributeactiondialog.h			\
		qgsattributeaction.h				\
		qgsattributedialog.h				\
		qgsattributetabledisplay.h			\
		qgsattributetable.h				\
		qgsbookmarkitem.h			\
		qgsbookmarks.h			\
		qgsclipboard.h					\
		qgsclipper.h						\
		qgscolortable.h						\
		qgscontcoldialog.h				\
		qgscontexthelp.h				\
		qgscontinuouscolrenderer.h			\
		qgscoordinatetransform.h			\
		qgscsexception.h				\
		qgscustomprojectiondialog.h			\
		qgscustomsymbol.h				\
		qgsdatamanager.h				\
		qgsdataprovider.h				\
		qgsdatasource.h					\
		qgsdatasourceuri.h				\
		qgsdelattrdialog.h				\
		qgsdistancearea.h				\
		qgsencodingfiledialog.h				\
		qgsexception.h					\
		qgsfeatureattribute.h				\
		qgsfeature.h						\
		qgsfield.h							\
		qgsfile.h					\
		qgsfillstylewidget.h				\
		qgsgeometry.h						\
		qgsgeometryvertexindex.h			\
		qgsgeomtypedialog.h				\
		qgsgraduatedsymrenderer.h			\
		qgsgrasydialog.h				\
		qgshelpviewer.h					\
		qgsidentifyresults.h				\
		qgslabelattributes.h				\
		qgslabeldialog.h				\
		qgslabel.h							\
		qgslayerprojectionselector.h \
		qgsline.h					\
		qgslinestyledialog.h				\
		qgslinesymbol.h					\
		qgsludialog.h					\
		qgsmapcanvas.h						\
		qgsmaplayer.h						\
		qgsmaplayerinterface.h				\
		qgsmaplayerregistry.h				\
		qgsmapoverviewcanvas.h				\
		qgsmapserverexport.h				\
		qgsmaptopixel.h					\
		qgsmarkercatalogue.h				\
		qgsmarkerdialog.h				\
		qgsmarkersymbol.h					\
		qgsmeasure.h					\
		qgsmessageviewer.h					\
		qgsnewhttpconnection.h				\
		qgsnumericsortlistviewitem.h			\
		qgsoptions.h					\
		qgspastetransformations.h			\
		qgspatterndialog.h				\
		qgspluginitem.h					\
		qgspluginmanager.h				\
		qgspluginmetadata.h				\
		qgspluginregistry.h				\
		qgspoint.h							\
		qgspointstylewidget.h				\
		qgspolygonsymbol.h				\
		qgsproject.h					\
		qgsprojectproperties.h			\
		qgsprojectproperty.h				\
		qgsprovidercountcalcevent.h			\
		qgsproviderextentcalcevent.h		\
		qgsprovidermetadata.h			\
		qgsproviderregistry.h				\
		qgsrangerenderitem.h				\
		qgsrasterdataprovider.h			\
		qgsrasterlayerproperties.h		\
		qgsrect.h							\
		qgsrenderer.h						\
		qgsrenderitem.h						\
		qgsrunprocess.h						\
		qgsscalecalculator.h				\
        qgssearchstring.h				\
        qgssearchtreenode.h				\
		qgsserversourceselect.h				\
		qgssinglesymrenderer.h				\
		qgssisydialog.h					\
		qgsspatialrefsys.h					\
		qgssvgcache.h					\
		qgssymbol.h							\
		qgssymbologyutils.h				\
		qgsuniquevalrenderer.h				\
		qgsuvaldialog.h					\
		qgsvectordataprovider.h				\
		qgsvectorfilewriter.h				\
		qgsvectorlayerproperties.h			\
		$(WIDGETPATH)/projectionselector/qgsprojectionselector.h 

## files generated from MOC
libqgis_la_MOC =							\
		qgisapp.moc.cpp						\
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
		qgscoordinatetransform.moc.cpp		\
		qgscustomprojectiondialog.moc.cpp 	\
		qgsdataprovider.moc.cpp				\
		qgsdelattrdialog.moc.cpp		\
		qgsencodingfiledialog.moc.cpp		\
		qgsfillstylewidget.moc.cpp			\
		qgsgeomtypedialog.moc.cpp		\
		qgsgrasydialog.moc.cpp			\
		qgshelpviewer.moc.cpp			\
		qgsidentifyresults.moc.cpp		\
		qgslabeldialog.moc.cpp			\
		qgslayerprojectionselector.moc.cpp \
		qgslinestyledialog.moc.cpp		\
		qgslinestylewidget.moc.cpp			\
		qgsludialog.moc.cpp			\
		qgsmapcanvas.moc.cpp				\
		qgsmaplayerinterface.moc.cpp		\
		qgsmaplayer.moc.cpp					\
		qgsmaplayerregistry.moc.cpp			\
		qgsmapoverviewcanvas.moc.cpp		\
		qgsmapserverexport.moc.cpp		\
		qgsmarkerdialog.moc.cpp			\
		qgsmeasure.moc.cpp			\
		qgsnewhttpconnection.moc.cpp		\
		qgsoptions.moc.cpp			\
		qgspastetransformations.moc.cpp		\
		qgspatterndialog.moc.cpp		\
		qgspluginmanager.moc.cpp		\
		qgspointstylewidget.moc.cpp			\
		qgsprojectproperties.moc.cpp		\
		qgsrasterdataprovider.moc.cpp		\
		qgsrasterlayerproperties.moc.cpp	\
		qgsrunprocess.moc.cpp				\
		qgsserversourceselect.moc.cpp		\
		qgssisydialog.moc.cpp			\
		qgsuvaldialog.moc.cpp			\
		qgsvectorlayer.moc.cpp \
		qgsvectorlayerproperties.moc.cpp	\
		$(WIDGETPATH)/projectionselector/qgsprojectionselector.moc.cpp

libqgis_la_SOURCES =						\
		qgisapp.cpp							\
		qgisiface.cpp			\
		qgisinterface.cpp		\
		qgsabout.cpp		\
		qgsacetatelines.cpp					\
		qgsacetateobject.cpp				\
		qgsacetaterectangle.cpp				\
		qgsaddattrdialog.cpp		\
		qgsadvancedattrsearch.cpp	\
		qgsattributeaction.cpp				\
		qgsattributeactiondialog.cpp	\
		qgsattributedialog.cpp		\
		qgsattributetable.cpp		\
		qgsattributetabledisplay.cpp	\
		qgsbookmarkitem.cpp	\
		qgsbookmarks.cpp	\
		qgsclipboard.cpp		\
		qgsclipper.cpp						\
		qgscolortable.cpp					\
		qgscontcoldialog.cpp		\
		qgscontexthelp.cpp		\
		qgscontinuouscolrenderer.cpp		\
		qgscoordinatetransform.cpp			\
		qgscustomprojectiondialog.cpp			\
		qgscustomsymbol.cpp		\
		qgsdatamanager.cpp				\
		qgsdatasource.cpp		\
		qgsdelattrdialog.cpp		\
		qgsdistancearea.cpp			\
		qgsencodingfiledialog.cpp	\
		qgsexception.cpp				\
		qgsfeatureattribute.cpp				\
		qgsfeature.cpp					\
		qgsfield.cpp						\
		qgsfile.cpp			\
		qgsfillstylewidget.cpp				\
		qgsgeometry.cpp						\
		qgsgeometryvertexindex.cpp			\
		qgsgeomtypedialog.cpp		\
		qgsgraduatedsymrenderer.cpp			\
		qgsgrasydialog.cpp		\
		qgshelpviewer.cpp		\
		qgsidentifyresults.cpp		\
		qgslabelattributes.cpp		\
		qgslabel.cpp						\
		qgslabeldialog.cpp		\
		qgslayerprojectionselector.cpp \
		qgsline.cpp			\
		qgslinestyledialog.cpp		\
		qgslinestylewidget.cpp				\
		qgslinesymbol.cpp		\
		qgsludialog.cpp			\
		qgsmapcanvas.cpp					\
		qgsmaplayer.cpp						\
		qgsmaplayerregistry.cpp				\
		qgsmapoverviewcanvas.cpp			\
		qgsmapserverexport.cpp		\
		qgsmaptopixel.cpp					\
		qgsmarkercatalogue.cpp				\
		qgsmarkerdialog.cpp		\
		qgsmarkersymbol.cpp					\
		qgsmeasure.cpp			\
		qgsmessageviewer.cpp				\
		qgsnewhttpconnection.cpp	\
		qgsnumericsortlistviewitem.cpp			\
		qgsoptions.cpp			\
		qgspastetransformations.cpp	\
		qgspatterndialog.cpp		\
		qgspluginitem.cpp		\
		qgspluginmanager.cpp		\
		qgspluginmetadata.cpp		\
		qgspluginregistry.cpp		\
		qgspoint.cpp						\
		qgspointstylewidget.cpp				\
		qgspolygonsymbol.cpp		\
		qgsproject.cpp			\
		qgsprojectproperties.cpp	\
		qgsprojectproperty.cpp				\
		qgsprovidercountcalcevent.cpp		\
		qgsproviderextentcalcevent.cpp		\
		qgsprovidermetadata.cpp		\
		qgsproviderregistry.cpp				\
		qgsrangerenderitem.cpp				\
		qgsrasterdataprovider.cpp	\
		qgsrasterlayerproperties.cpp	\
		qgsrect.cpp							\
		qgsrenderer.cpp						\
		qgsrenderitem.cpp					\
		qgsrunprocess.cpp					\
		qgsscalecalculator.cpp				\
        qgssearchstring.cpp             \
        qgssearchstringlexer.ll		\
        qgssearchstringparser.yy	\
        qgssearchtreenode.cpp		\
		qgsserversourceselect.cpp	\
		qgssinglesymrenderer.cpp			\
		qgssisydialog.cpp		\
		qgsspatialrefsys.cpp				\
		qgssvgcache.cpp			\
		qgssymbol.cpp						\
		qgssymbologyutils.cpp		\
		qgsuniquevalrenderer.cpp			\
		qgsuvaldialog.cpp		\
		qgsvectordataprovider.cpp			\
		qgsvectorfilewriter.cpp				\
		qgsvectorlayer.cpp \
		qgsvectorlayerproperties.cpp	\
		$(WIDGETPATH)/projectionselector/qgsprojectionselector.cpp

if HAVE_POSTGRESQL
postgresHEADERS = qgsdbsourceselect.h		\
		  qgsnewconnection.h		\
		  qgspgquerybuilder.h

postgresSOURCES = qgsdbsourceselect.cpp		\
		  qgsnewconnection.cpp		\
		  qgspgquerybuilder.cpp

postgresMOC = qgsdbsourceselect.moc.cpp		\
		  qgsnewconnection.moc.cpp		\
		  qgspgquerybuilder.moc.cpp
		  
postgresLIBHEADERS = qgspgutil.h
postgresLIBSOURCES = qgspgutil.cpp

libqgis_la_SOURCES += $(postgresSOURCES) $(postgresLIBSOURCES) $(postgresMOC)
endif

AM_YFLAGS       = -d
qgis_YACC       = qgssearchstringparser.h


nodist_libqgis_la_SOURCES = $(libqgis_la_MOC)  

BUILT_SOURCES = $(libqgis_la_MOC) $(qgis_YACC)  $(postgresMOC)


libqgis_la_LIBADD = raster/libqgis_raster.la legend/libqgis_legend.la composer/libqgis_composer.la $(QT_LDADD) $(GEOS_LDADD) $(GDAL_LDADD) $(PYTHON_LIB) -lsqlite3
libqgis_la_LDFLAGS = -version-info $(INTERFACE_VERSION)

libqgis_la_CXXFLAGS = $(PREFIX) $(PLUGINPATH) $(PKGDATAPATH) $(GDAL_CFLAGS) $(CXXFLAGS) $(EXTRA_CXXFLAGS) $(QT_CXXFLAGS) $(PG_INC) $(DEBUG_QGIS) $(GEOS_CFLAGS) $(PYTHON_INCLUDE_DIR) $(HAVE_PYTHON) -I./ui/ -I../widgets/projectionselector/ -I./legend/ -I./raster/ -I./composer/

## for installing headers in $(includedir)/qgis
pkginclude_HEADERS = $(qgisHEADERS) 

pkginclude_dir = $(includedir)/qgis


CLEANFILES = $(BUILT_SOURCES)

EXTRA_DIST=

if HAVE_POSTGRESQL

EXTRA_DIST += $(postgresSOURCES) \
		$(postgresLIBSOURCES) 
		
pkginclude_HEADERS += $(postgresHEADERS) \
		$(postgresLIBHEADERS) \
		$(libqgisHEADERS) 
endif

noinst_HEADERS = qgsdatasourceuri.h
