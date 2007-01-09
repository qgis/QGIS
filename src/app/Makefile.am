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



##
## ----------------------------------------------------------------------
##         QGIS Desktop Application
## ----------------------------------------------------------------------
##

qgis_SOURCES = main.cpp $(qgisSOURCES) $(qgisMOC)

qgis_LDADD = $(LDADD) $(QT_LDADD) $(GDAL_LDADD) $(PG_LIB) $(GEOS_LDADD) $(PYTHON_LIB) -lproj -lsqlite3 ../gui/libqgis_gui.la ../core/libqgis_core.la
if HAVE_QTMAC
qgis_LDFLAGS = -framework ApplicationServices
else
qgis_LDFLAGS = -rdynamic
endif
qgis_CXXFLAGS = $(PREFIX) $(PLUGINPATH) $(PKGDATAPATH) $(GDAL_CFLAGS) $(CXXFLAGS) $(EXTRA_CXXFLAGS) $(QT_CXXFLAGS) $(PG_INC) $(DEBUG_QGIS)  $(HAVE_PYTHON) $(GEOS_CFLAGS) $(PYTHON_INCLUDE_DIR) -Icomposer -Ilegend -I../ui/ -I../core/include/ -I../gui/ -I../plugins/

##
## ----------------------------------------------------------------------
##         QGIS Legend
## ----------------------------------------------------------------------
##


legendMOC = \
             legend/qgslegend.moc.cpp \
             legend/qgslegendlayerfile.moc.cpp

legendSOURCES = \
			  legend/qgslegendgroup.cpp \
        legend/qgslegend.cpp \
        legend/qgslegenditem.cpp \
        legend/qgslegendlayer.cpp \
        legend/qgslegendlayerfile.cpp \
        legend/qgslegendlayerfilegroup.cpp \
        legend/qgslegendpropertygroup.cpp \
        legend/qgslegendpropertyitem.cpp \
        legend/qgslegendsymbologygroup.cpp \
        legend/qgslegendsymbologyitem.cpp \
        legend/qgslegendvectorsymbologyitem.cpp

legendHEADERS =


##
## ----------------------------------------------------------------------
##         QGIS Composer
## ----------------------------------------------------------------------
##



composerMOC = \
    composer/qgscomposer.moc.cpp  \
    composer/qgscomposerlabel.moc.cpp      \
    composer/qgscomposerpicture.moc.cpp      \
    composer/qgscomposermap.moc.cpp        \
    composer/qgscomposerscalebar.moc.cpp     \
    composer/qgscomposervectorlegend.moc.cpp   \
    composer/qgscomposerview.moc.cpp       \
    composer/qgscomposition.moc.cpp

composerHEADERS =
##composerHEADERS = \
##    composer/qgscomposer.h \
##    composer/qgscomposeritem.h         \
##    composer/qgscomposerlabel.h          \
##    composer/qgscomposerpicture.h        \
##    composer/qgscomposermap.h          \
##    composer/qgscomposerscalebar.h       \
##    composer/qgscomposervectorlegend.h     \
##    composer/qgscomposerview.h         \
##    composer/qgscomposition.h

composerSOURCES = \
		composer/qgscomposer.cpp \
    composer/qgscomposeritem.cpp         \
    composer/qgscomposerlabel.cpp        \
    composer/qgscomposerpicture.cpp        \
    composer/qgscomposermap.cpp          \
    composer/qgscomposerscalebar.cpp       \
    composer/qgscomposervectorlegend.cpp     \
    composer/qgscomposerview.cpp         \
    composer/qgscomposition.cpp


##
## ----------------------------------------------------------------------
##         QGIS app stuff starts here
## ----------------------------------------------------------------------
##

qgisHEADERS =							\
		qgisapp.h							\
		qgisappinterface.h					\
		qgsabout.h				\
		qgsaddattrdialog.h				\
		qgsattributeactiondialog.h			\
		qgsattributedialog.h				\
		qgsattributetabledisplay.h			\
		qgsattributetable.h				\
		qgsbookmarkitem.h			\
		qgsbookmarks.h			\
		qgsclipboard.h				\
		qgscolorbutton.h				\
		qgscontinuouscolordialog.h				\
		qgscustomprojectiondialog.h			\
		qgsdelattrdialog.h				\
		qgsencodingfiledialog.h				\
		qgsfillstylewidget.h				\
		qgsgeomtypedialog.h				\
		qgsgraduatedsymboldialog.h				\
		qgshelpviewer.h					\
		qgsidentifyresults.h				\
		qgslabeldialog.h				\
		qgslayerprojectionselector.h \
		qgslinestyledialog.h				\
		qgsludialog.h					\
		qgsmaplayerinterface.h				\
		qgsmapserverexport.h				\
		qgsmaptoolcapture.h				\
		qgsmaptoolidentify.h			\
		qgsmaptoolselect.h				\
		qgsmaptoolvertexedit.h			\
		qgsmarkerdialog.h				\
		qgsmeasure.h					\
		qgsnewhttpconnection.h				\
		qgsnumericsortlistviewitem.h		\
		qgsoptions.h					\
		qgspastetransformations.h			\
		qgspatterndialog.h				\
		qgspluginitem.h					\
		qgspluginmanager.h				\
		qgspluginmetadata.h				\
		qgspluginregistry.h				\
		qgspointstylewidget.h				\
		qgsprojectproperties.h			\
		qgsrasterlayerproperties.h		\
		qgssearchquerybuilder.h			\
		qgsserversourceselect.h				\
		qgssinglesymboldialog.h				\
		qgsuniquevaluedialog.h					\
		qgsvectorlayerproperties.h			\
		$(legendHEADERS) \
		$(composerHEADERS)


## files generated from MOC
qgisMOC =							\
		qgisapp.moc.cpp						\
		qgisappinterface.moc.cpp \
		qgsabout.moc.cpp		\
		qgsaddattrdialog.moc.cpp		\
		qgsattributeactiondialog.moc.cpp	\
		qgsattributedialog.moc.cpp		\
		qgsattributetabledisplay.moc.cpp		\
		qgsattributetable.moc.cpp		\
		qgsbookmarks.moc.cpp		\
		qgscontinuouscolordialog.moc.cpp		\
		qgscustomprojectiondialog.moc.cpp 	\
		qgsdelattrdialog.moc.cpp		\
		qgsencodingfiledialog.moc.cpp		\
		qgsfillstylewidget.moc.cpp			\
		qgsgeomtypedialog.moc.cpp		\
		qgsgraduatedsymboldialog.moc.cpp			\
		qgshelpviewer.moc.cpp			\
		qgsidentifyresults.moc.cpp		\
		qgslabeldialog.moc.cpp			\
		qgslayerprojectionselector.moc.cpp \
		qgslinestyledialog.moc.cpp		\
		qgslinestylewidget.moc.cpp			\
		qgsludialog.moc.cpp			\
		qgsmaplayerinterface.moc.cpp		\
		qgsmapserverexport.moc.cpp		\
		qgsmaptoolidentify.moc.cpp		\
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
		qgssearchquerybuilder.moc.cpp	\
		qgsserversourceselect.moc.cpp		\
		qgssinglesymboldialog.moc.cpp			\
		qgsuniquevaluedialog.moc.cpp			\
		qgsvectorlayerproperties.moc.cpp	\
		$(legendMOC) \
		$(composerMOC)

qgisSOURCES =						\
		qgisapp.cpp							\
		qgisappinterface.cpp			\
		qgsabout.cpp		\
		qgsaddattrdialog.cpp		\
		qgsattributeactiondialog.cpp	\
		qgsattributedialog.cpp		\
		qgsattributetable.cpp		\
		qgsattributetabledisplay.cpp	\
		qgsbookmarkitem.cpp		\
		qgsbookmarks.cpp	\
		qgsclipboard.cpp		\
		qgscolorbutton.cpp		\
		qgscontinuouscolordialog.cpp		\
		qgscustomprojectiondialog.cpp			\
		qgsdelattrdialog.cpp		\
		qgsencodingfiledialog.cpp	\
		qgsfillstylewidget.cpp				\
		qgsgeomtypedialog.cpp		\
		qgsgraduatedsymboldialog.cpp		\
		qgshelpviewer.cpp		\
		qgsidentifyresults.cpp		\
		qgslabeldialog.cpp		\
		qgslayerprojectionselector.cpp \
		qgslinestyledialog.cpp		\
		qgslinestylewidget.cpp				\
		qgsludialog.cpp			\
		qgsmapserverexport.cpp		\
		qgsmaptoolcapture.cpp			\
		qgsmaptoolidentify.cpp		\
		qgsmaptoolselect.cpp			\
		qgsmaptoolvertexedit.cpp		\
		qgsmarkerdialog.cpp		\
		qgsmeasure.cpp			\
		qgsnewhttpconnection.cpp	\
		qgsnumericsortlistviewitem.cpp		\
		qgsoptions.cpp			\
		qgspastetransformations.cpp	\
		qgspatterndialog.cpp		\
		qgspluginitem.cpp				\
		qgspluginmanager.cpp		\
		qgspluginmetadata.cpp		\
		qgspluginregistry.cpp		\
		qgspointstylewidget.cpp				\
		qgsprojectproperties.cpp	\
		qgsrasterlayerproperties.cpp	\
		qgssearchquerybuilder.cpp		\
		qgsserversourceselect.cpp	\
		qgssinglesymboldialog.cpp	\
		qgsuniquevaluedialog.cpp		\
		qgsvectorlayerproperties.cpp		\
		$(legendSOURCES) \
		$(composerSOURCES)

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
		  
qgisSOURCES += $(postgresSOURCES) $(postgresMOC)
endif


BUILT_SOURCES = $(qgisMOC) $(postgresMOC)


##
## ----------------------------------------------------------------------
##         additional files that need to be installed go here
## ----------------------------------------------------------------------
##

## don't install headers
##pkginclude_HEADERS = $(qgisHEADERS) 
##pkginclude_dir = $(includedir)/qgis


CLEANFILES = $(BUILT_SOURCES)

EXTRA_DIST=

if HAVE_POSTGRESQL

EXTRA_DIST += $(postgresSOURCES)
		
##pkginclude_HEADERS += $(postgresHEADERS)

endif

