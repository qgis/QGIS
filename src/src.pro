######################################################################
# Qmake project file for QGIS src directory
# This file is used by qmake to generate the Makefile for building
# QGIS on Windows
#
# src.pro,v 1.55 2004/10/15 21:47:19 gsherman Exp 
######################################################################
#
# For a WIN32 release build do the following:
# 1. comment out the DEFINES += QGISDEBUG line 
# 2. remove console and debug from the CONFIG += line
# 3. qmake
# 4. nmake clean
# 5. nmake
#
######################################################################

################# 
# GEOS Notes    #
###########################################################################
# Geos support is currenlty provided by a custom compiled library.        #
# The library is compiled with vc++ and statically linked with the        #
# ogr provider. A dll may be supplied at a later date. GEOS source        #
# used in creating the library is available at geos.refractions.net.      #
# To compile the windows version, set the GEOS environment variable       #
# to point to the directory containing the include and lib subdirectories.#
# The headers/lib can be downloaded from http://qgis.org/win32_geos.zip   #
###########################################################################

TEMPLATE	= app
LANGUAGE	= C++

CONFIG	+= qt thread rtti debug console

LIBS	+= $(GDAL)\lib\gdal_i.lib $(POSTGRESQL)\src\interfaces\libpq\Release\libpq.lib $(GEOS)\lib\geos.lib

DEFINES	+= QGISDEBUG

INCLUDEPATH	+= . $(GDAL)\include $(POSTGRESQL)\src\interfaces\libpq $(POSTGRESQL)\src\include $(GEOS)\include

TARGET = qgis
DESTDIR = ../win_build
RC_FILE = qgis_win32.rc

# Input
HEADERS +=  legend/qgslegend.h \
            legend/qgslegendgroup.h \
            legend/qgslegenditem.h \
            legend/qgslegendlayer.h \
            legend/qgslegendlayerfile.h \
            legend/qgslegendpropertygroup.h \
            legend/qgslegendpropertyitem.h \
            legend/qgslegendsymbologygroup.h \
            legend/qgslegendsymbologyitem.h \
            legend/qgslegendvectorsymbologyitem.h \
            qgis.h \
            qgisapp.h \
            qgisappbase.ui.h \
            qgisiface.h \
            qgisinterface.h \
            qgsabout.ui.h \
            qgsattributeaction.h \
            qgsattributeactiondialog.h \
            qgsattributetable.h \
            qgsattributetablebase.ui.h \
            qgsattributetabledisplay.h \
            qgsattributedialog.h \
            qgscolortable.h \
            qgscontcoldialog.h \
            qgscontinuouscolrenderer.h \
            qgscoordinatetransform.h \
            qgscustomsymbol.h \
            qgsdataprovider.h \
            qgsdatasource.h \
            qgsdbsourceselect.h \
            qgsdbsourceselectbase.ui.h \
            qgsdlgvectorlayerproperties.h \
            qgsfeature.h \
            qgsfeatureattribute.h \
            qgsfield.h \
            qgsgeomtypedialog.h \
            qgsgraduatedsymrenderer.h \
            qgsgrasydialog.h \
            qgshelpviewer.h \
            qgshelpviewerbase.ui.h \
            qgsidentifyresults.h \
            qgslinestyledialog.h \
            qgslinesymbol.h \
            qgsmapcanvas.h \
            qgsmaplayer.h \
            qgsmaplayerinterface.h \
            qgsmaplayerregistry.h \
            qgsmapserverexport.h \
            qgsmapserverexportbase.ui.h \
            qgsmarkerdialog.h \
            qgsmarkersymbol.h \
            qgsmessageviewer.ui.h \
            qgsnewconnection.h \
            qgsoptions.h \
            qgsoptionsbase.ui.h \
            qgspatterndialog.h \
            qgspgquerybuilder.h \
            qgspluginitem.h \
            qgspluginmanager.h \
            qgspluginmetadata.h \
            qgspluginregistry.h \
            qgspoint.h \
            qgspolygonsymbol.h \
            qgsproject.h \
            qgsprojectproperties.h \
            qgsprojectpropertiesbase.ui.h \
            qgsprovidermetadata.h \
            qgsproviderregistry.h \
            qgsrangerenderitem.h \
            qgsrasterlayer.h \
            qgsrasterlayerproperties.h \
            qgsrasterlayerpropertiesbase.ui.h \
            qgsrect.h \
            qgsrenderer.h \
            qgsrenderitem.h \
            qgsscalecalculator.h \
            qgssinglesymrenderer.h \
            qgssisydialog.h \
            qgssymbol.h \
            qgssymbologyutils.h \
            qgstable.h \
            qgsvectordataprovider.h \
            qgsvectorlayer.h \
            qgssvgcache.h \
            qgsacetateobject.h \
            qgslabel.h \
            qgslabelattributes.h \
            qgslabeldialog.h \
            qgsacetaterectangle.h \
            qgsuvaldialog.h \
            qgsludialog.h \
            qgsuniquevalrenderer.h \
            qgsvectorfilewriter.h \
            splashscreen.h

INTERFACES += qgisappbase.ui \
            qgsabout.ui \
            qgsattributetablebase.ui \
            qgsattributeactiondialogbase.ui \
            qgsattributedialogbase.ui \
            qgscontcoldialogbase.ui \
            qgsdbsourceselectbase.ui \
            qgsdlgvectorlayerpropertiesbase.ui \
            qgsgramadialogbase.ui \
            qgsgrasydialogbase.ui \
            qgshelpviewerbase.ui \
            qgsidentifyresultsbase.ui \
            qgslegenditembase.ui \
            qgslinestyledialogbase.ui \
            qgsmapserverexportbase.ui \
            qgsmarkerdialogbase.ui \
            qgsmessageviewer.ui \
            qgsnewconnectionbase.ui \
            qgsoptionsbase.ui \
            qgspatterndialogbase.ui \
            qgspgquerybuilderbase.ui \
            qgspluginmanagerbase.ui \
            qgsprojectpropertiesbase.ui \
            qgsrasterlayerpropertiesbase.ui \
            qgslabeldialogbase.ui \ 
            qgssimadialogbase.ui \
            qgssisydialogbase.ui \ 
            qgsludialogbase.ui \
            qgsuvaldialogbase.ui \
            qgsuvalmadialogbase.ui \
            qgsgeomtypedialog.ui

SOURCES +=  legend/qgslegend.cpp \
            legend/qgslegendgroup.cpp \
            legend/qgslegenditem.cpp \
            legend/qgslegendlayer.cpp \
            legend/qgslegendlayerfile.cpp \
            legend/qgslegendpropertygroup.cpp \
            legend/qgslegendpropertyitem.cpp \
            legend/qgslegendsymbologygroup.cpp \
            legend/qgslegendsymbologyitem.cpp \
            legend/qgslegendvectorsymbologyitem.cpp \
            main.cpp \
            qgisapp.cpp \
            qgisiface.cpp \
            qgisinterface.cpp \
            qgsattributeaction.cpp \
            qgsattributeactiondialog.cpp \
            qgsattributedialog.cpp \
            qgsattributetable.cpp \
            qgsattributetabledisplay.cpp \
            qgscolortable.h \
            qgscontcoldialog.cpp \
            qgscontinuouscolrenderer.cpp \
            qgscoordinatetransform.cpp \
            qgscustomsymbol.cpp \
            qgsdatasource.cpp \
            qgsdbsourceselect.cpp \
            qgsdlgvectorlayerproperties.cpp \
            qgsfeature.cpp \
            qgsfeatureattribute.cpp \
            qgsfield.cpp \
            qgsgeomtypedialog.cpp \
            qgsgraduatedsymrenderer.cpp \
            qgsgrasydialog.cpp \
            qgshelpviewer.cpp \
            qgsidentifyresults.cpp \
            qgslinestyledialog.cpp \
            qgslinesymbol.cpp \
            qgsmapcanvas.cpp \
            qgsmaplayer.cpp \
            qgsmaplayerregistry.cpp \
            qgsmapserverexport.cpp \
            qgsmarkerdialog.cpp \
            qgsmarkersymbol.cpp \
            qgsnewconnection.cpp \
            qgsoptions.cpp \
            qgspatterndialog.cpp \
            qgspgquerybuilder.cpp \
            qgspluginitem.cpp \
            qgspluginmanager.cpp \
            qgspluginmetadata.cpp \
            qgspluginregistry.cpp \
            qgspoint.cpp \
            qgspolygonsymbol.cpp \
            qgsproject.cpp \
            qgsprojectproperties.cpp \
            qgsprovidermetadata.cpp \
            qgsproviderregistry.cpp \
            qgsrangerenderitem.cpp \
            qgsrasterlayer.cpp \
            qgsrasterlayerproperties.cpp \
            qgsrect.cpp \
            qgsrenderitem.cpp \
            qgsscalecalculator.cpp \
            qgssinglesymrenderer.cpp \
            qgssisydialog.cpp \
            qgssymbol.cpp \
            qgssymbologyutils.cpp \
            qgsvectorlayer.cpp \
            qgssvgcache.cpp \
            qgsacetateobject.cpp \
            qgslabeldialog.cpp \
            qgslabel.cpp \
            qgslabelattributes.cpp \
            qgsacetaterectangle.cpp \
            qgsuvaldialog.cpp \
            qgsludialog.cpp \
            qgsuniquevalrenderer.cpp \
            qgsvectorfilewriter.cpp \
            splashscreen.cpp

FORMS	= qgisappbase.ui \
	qgsabout.ui \
	qgsattributetablebase.ui \
	qgsattributeactiondialogbase.ui \
	qgsattributedialogbase.ui \
	qgscontcoldialogbase.ui \
	qgsdbsourceselectbase.ui \
	qgsdlgvectorlayerpropertiesbase.ui \
	qgsgrasydialogbase.ui \
	qgshelpviewerbase.ui \
	qgsidentifyresultsbase.ui \
	qgslabeldialogbase.ui \
	qgslinestyledialogbase.ui \
	qgsmapserverexportbase.ui \
	qgsmarkerdialogbase.ui \
	qgsmessageviewer.ui \
	qgsnewconnectionbase.ui \
	qgsoptionsbase.ui \
	qgspatterndialogbase.ui \
	qgspgquerybuilderbase.ui \
	qgspluginmanagerbase.ui \
	qgsprojectpropertiesbase.ui \
	qgsrasterlayerpropertiesbase.ui
