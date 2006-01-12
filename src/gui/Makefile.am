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

if !HAVE_QTMAC
PREFIX=-DPREFIX=\"$(prefix)\"
PLUGINPATH=-DPLUGINPATH=\"$(pkglibdir)\"
PKGDATAPATH=-DPKGDATAPATH=\"$(pkgdatadir)\"
endif

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

lib_LTLIBRARIES = libqgis_gui.la


##
## ----------------------------------------------------------------------
##         QGIS Desktop Application
## ----------------------------------------------------------------------
##

qgis_SOURCES =	main.cpp

qgis_LDADD = ../raster/libqgis_raster.la ../legend/libqgis_legend.la ../composer/libqgis_composer.la $(LDADD) $(QT_LDADD) $(GDAL_LDADD) $(PG_LIB) $(GEOS_LDADD) $(PYTHON_LIB) -lproj -lsqlite3 libqgis_gui.la ../core/libqgis_core.la
if HAVE_QTMAC
qgis_LDFLAGS = -framework ApplicationServices
else
qgis_LDFLAGS = -rdynamic
endif
qgis_CXXFLAGS = $(PREFIX) $(PLUGINPATH) $(PKGDATAPATH) $(GDAL_CFLAGS) $(CXXFLAGS) $(EXTRA_CXXFLAGS) $(QT_CXXFLAGS) $(PG_INC) $(DEBUG_QGIS)  $(HAVE_PYTHON) $(GEOS_CFLAGS) $(PYTHON_INCLUDE_DIR) -I../ui/ -I../widgets/projectionselector/ -I../legend/ -I../raster/ -I../composer/ -I../core/ -I../plugins/


##
## ----------------------------------------------------------------------
##         libqgis_gui-gui stuff starts here
## ----------------------------------------------------------------------
##

libqgis_guiHEADERS =							\
		qgisapp.h							\
		qgisiface.h					\
		qgisinterface.h \
		qgsabout.h				\
		qgsaddattrdialog.h				\
		qgsadvancedattrsearch.h				\
		qgsattributeaction.h		\
		qgsattributeactiondialog.h			\
		qgsattributedialog.h				\
		qgsattributetabledisplay.h			\
		qgsattributetable.h				\
		qgsbookmarks.h			\
		qgsclipboard.h				\
		qgscontcoldialog.h				\
		qgscontinuouscolrenderer.h			\
		qgscsexception.h				\
		qgscoordinatetransform.h	\
		qgscustomprojectiondialog.h			\
		qgsdelattrdialog.h				\
		qgsencodingfiledialog.h				\
		qgsfeature.h						\
		qgsfillstylewidget.h				\
		qgsgeomtypedialog.h				\
		qgsgraduatedsymrenderer.h			\
		qgsgrasydialog.h				\
		qgshelpviewer.h					\
		qgsidentifyresults.h				\
		qgslabeldialog.h				\
		qgslabel.h							\
		qgslayerprojectionselector.h \
		qgslinestyledialog.h				\
		qgsludialog.h					\
		qgsmapcanvas.h						\
		qgsmaplayer.h						\
		qgsmaplayerinterface.h				\
		qgsmaplayerregistry.h				\
		qgsmapoverviewcanvas.h				\
		qgsmapserverexport.h				\
		qgsmarkerdialog.h				\
		qgsmeasure.h					\
		qgsmessageviewer.h					\
		qgsnewhttpconnection.h				\
		qgsoptions.h					\
		qgspastetransformations.h			\
		qgspatterndialog.h				\
		qgspluginmanager.h				\
		qgspluginmetadata.h				\
		qgspointstylewidget.h				\
		qgsproject.h					\
		qgsprojectproperties.h			\
		qgsrangerenderitem.h				\
		qgsrasterlayerproperties.h		\
		qgsrubberband.h						\
		qgsrunprocess.h						\
		qgsserversourceselect.h				\
		qgssisydialog.h					\
		qgsspatialrefsys.h					\
		qgsuniquevalrenderer.h				\
		qgsuvaldialog.h					\
		qgsvectorfilewriter.h				\
		qgsvectorlayerproperties.h			

## files generated from MOC
libqgis_gui_la_MOC =							\
		qgisapp.moc.cpp						\
		qgisinterface.moc.cpp	\
		qgsabout.moc.cpp		\
		qgsaddattrdialog.moc.cpp		\
		qgsadvancedattrsearch.moc.cpp		\
		qgsattributeactiondialog.moc.cpp	\
		qgsattributedialog.moc.cpp		\
		qgsattributetabledisplay.moc.cpp		\
		qgsattributetable.moc.cpp		\
		qgsbookmarks.moc.cpp		\
		qgscontcoldialog.moc.cpp		\
		qgscoordinatetransform.moc.cpp		\
		qgscustomprojectiondialog.moc.cpp 	\
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
		qgsrasterlayerproperties.moc.cpp	\
		qgsrunprocess.moc.cpp				\
		qgsserversourceselect.moc.cpp		\
		qgssisydialog.moc.cpp			\
		qgsuvaldialog.moc.cpp			\
		qgsvectorlayer.moc.cpp \
		qgsvectorlayerproperties.moc.cpp	

libqgis_gui_la_SOURCES =						\
		qgisapp.cpp							\
		qgisiface.cpp			\
		qgisinterface.cpp	\
		qgsabout.cpp		\
		qgsaddattrdialog.cpp		\
		qgsadvancedattrsearch.cpp	\
	   	qgsattributeaction.cpp				\
		qgsattributeactiondialog.cpp	\
		qgsattributedialog.cpp		\
		qgsattributetable.cpp		\
		qgsattributetabledisplay.cpp	\
		qgsbookmarks.cpp	\
		qgsclipboard.cpp		\
		qgscontcoldialog.cpp		\
		qgscoordinatetransform.cpp			\
		qgscontinuouscolrenderer.cpp		\
		qgscustomprojectiondialog.cpp			\
		qgsdelattrdialog.cpp		\
		qgsencodingfiledialog.cpp	\
		qgsfeature.cpp					\
		qgsfillstylewidget.cpp				\
		qgsgeomtypedialog.cpp		\
		qgsgraduatedsymrenderer.cpp			\
		qgsgrasydialog.cpp		\
		qgshelpviewer.cpp		\
		qgsidentifyresults.cpp		\
		qgslabel.cpp						\
		qgslabeldialog.cpp		\
		qgslayerprojectionselector.cpp \
		qgslinestyledialog.cpp		\
		qgslinestylewidget.cpp				\
		qgsludialog.cpp			\
		qgsmapcanvas.cpp					\
		qgsmaplayer.cpp						\
		qgsmaplayerregistry.cpp				\
		qgsmapoverviewcanvas.cpp			\
		qgsmapserverexport.cpp		\
		qgsmarkerdialog.cpp		\
		qgsmeasure.cpp			\
		qgsmessageviewer.cpp				\
		qgsnewhttpconnection.cpp	\
		qgsoptions.cpp			\
		qgspastetransformations.cpp	\
		qgspatterndialog.cpp		\
		qgspluginmanager.cpp		\
		qgspluginmetadata.cpp		\
		qgspointstylewidget.cpp				\
		qgsproject.cpp			\
		qgsprojectproperties.cpp	\
		qgsrasterlayerproperties.cpp	\
		qgsrubberband.cpp					\
		qgsrunprocess.cpp					\
		qgsserversourceselect.cpp	\
		qgssinglesymrenderer.cpp			\
		qgssisydialog.cpp		\
		qgsspatialrefsys.cpp				\
		qgsuniquevalrenderer.cpp			\
		qgsuvaldialog.cpp		\
		qgsvectorfilewriter.cpp				\
		qgsvectorlayer.cpp \
		qgsvectorlayerproperties.cpp	

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

libqgis_gui_la_SOURCES += $(postgresSOURCES) $(postgresLIBSOURCES) $(postgresMOC)
endif

AM_YFLAGS       = -d

nodist_libqgis_gui_la_SOURCES = $(libqgis_gui_la_MOC)

BUILT_SOURCES = $(libqgis_gui_la_MOC) $(qgis_YACC)  $(postgresMOC)


libqgis_gui_la_LIBADD = ../core/libqgis_core.la ../raster/libqgis_raster.la ../legend/libqgis_legend.la ../composer/libqgis_composer.la ../widgets/projectionselector/libqgsprojectionselector.la $(QT_LDADD) $(GEOS_LDADD) $(GDAL_LDADD) $(PYTHON_LIB) -lsqlite3
libqgis_gui_la_LDFLAGS = -version-info $(INTERFACE_VERSION)

libqgis_gui_la_CXXFLAGS = $(PREFIX) $(PLUGINPATH) $(PKGDATAPATH) $(GDAL_CFLAGS) $(CXXFLAGS) $(EXTRA_CXXFLAGS) $(QT_CXXFLAGS) $(PG_INC) $(DEBUG_QGIS) $(GEOS_CFLAGS) $(PYTHON_INCLUDE_DIR) $(HAVE_PYTHON) -I../ui/ -I../widgets/projectionselector/ -I../legend/ -I../raster/ -I../composer/ -I../core/ -I../plugins/

##
## ----------------------------------------------------------------------
##         additional files that need to be installed go here
## ----------------------------------------------------------------------
##
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

