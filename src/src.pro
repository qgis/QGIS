######################################################################
# Qmake project file for QGIS src directory
# This file is used by qmake to generate the Makefile for building
# QGIS on Windows
#
# src.pro,v 1.43 2004/08/18 03:15:55 gsherman Exp 
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

TEMPLATE = app
TARGET = qgis
INCLUDEPATH += . $(GDAL)\include \
                $(POSTGRESQL)\src\interfaces\libpq \
                $(POSTGRESQL)\src\include
LIBS += $(GDAL)\lib\gdal_i.lib \
        $(POSTGRESQL)\src\interfaces\libpq\Release\libpq.lib 

DEFINES+= QGISDEBUG
DESTDIR = ../win_build
CONFIG += qt thread rtti debug console
#CONFIG += qt thread rtti 
RC_FILE = qgis_win32.rc
# Input
HEADERS += qgis.h \
           qgisapp.h \
           qgisappbase.ui.h \
           qgisiface.h \
           qgisinterface.h \
           qgsabout.ui.h \
           qgsattributetable.h \
           qgsattributetablebase.ui.h \
           qgsattributetabledisplay.h \
           qgsconfig.h \
           qgscontcoldialog.h \
           qgscontinuouscolrenderer.h \
           qgscoordinatetransform.h \
           qgscustomsymbol.h \
           qgsdatabaselayer.h \
           qgsdataprovider.h \
           qgsdatasource.h \
           qgsdbsourceselect.h \
           qgsdbsourceselectbase.ui.h \
           qgsdlgvectorlayerproperties.h \
           qgsfeature.h \
           qgsfeatureattribute.h \
           qgsfield.h \
           qgsgraduatedmarenderer.h \
           qgsgraduatedsymrenderer.h \
           qgsgramadialog.h \
           qgsgramaextensionwidget.h \
           qgsgrasydialog.h \
           qgsgrasyextensionwidget.h \
           qgshelpviewer.h \
           qgshelpviewerbase.ui.h \
           qgsidentifyresults.h \
           qgslegend.h \
           qgslegenditem.h \
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
           qgspluginitem.h \
           qgspluginmanager.h \
           qgspluginmetadata.h \
           qgspluginregistry.h \
           qgspoint.h \
           qgspolygonsymbol.h \
           qgsprojectio.h \
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
           qgssimadialog.h \
           qgssimarenderer.h \
           qgssinglesymrenderer.h \
           qgssisydialog.h \
           qgssymbol.h \
           qgssymbologyutils.h \
           qgstable.h \
           qgsvectorlayer.h \
           qgssvgcache.h \
           qgsvectorlayerproperties.h \
           splashscreen.h \
	   qgsacetateobject.h \
           qgslabel.h \
           qgslabelattributes.h \
           qgslabeldialog.h \
	   qgsacetaterectangle.h \
     qgsuvaldialog.h \
     qgsuniquevalrenderer.cpp
INTERFACES += qgisappbase.ui \
              qgsabout.ui \
              qgsattributetablebase.ui \
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
              qgspluginmanagerbase.ui \
              qgsprojectpropertiesbase.ui \
              qgsrasterlayerpropertiesbase.ui \
              qgssimadialogbase.ui \
              qgssisydialogbase.ui \
              qgsvectorlayerpropertiesbase.ui \
              qgsuvaldialogbase.ui
SOURCES += main.cpp \
           qgisapp.cpp \
           qgisiface.cpp \
           qgisinterface.cpp \
           qgsattributetable.cpp \
           qgsattributetabledisplay.cpp \
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
           qgsgraduatedmarenderer.cpp \
           qgsgraduatedsymrenderer.cpp \
           qgsgramadialog.cpp \
           qgsgramaextensionwidget.cpp \
           qgsgrasydialog.cpp \
           qgsgrasyextensionwidget.cpp \
           qgshelpviewer.cpp \
           qgsidentifyresults.cpp \
           qgslegend.cpp \
           qgslegenditem.cpp \
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
           qgspluginitem.cpp \
           qgspluginmanager.cpp \
           qgspluginmetadata.cpp \
           qgspluginregistry.cpp \
           qgspoint.cpp \
           qgspolygonsymbol.cpp \
           qgsprojectio.cpp \
           qgsprojectproperties.cpp \
           qgsprovidermetadata.cpp \
           qgsproviderregistry.cpp \
           qgsrangerenderitem.cpp \
           qgsrasterlayer.cpp \
           qgsrasterlayerproperties.cpp \
           qgsrect.cpp \
           qgsrenderitem.cpp \
           qgsscalecalculator.cpp \
           qgssimadialog.cpp \
           qgssimarenderer.cpp \
           qgssinglesymrenderer.cpp \
           qgssisydialog.cpp \
           qgssymbol.cpp \
           qgssymbologyutils.cpp \
           qgsvectorlayer.cpp \
           qgssvgcache.cpp \
           qgsvectorlayerproperties.cpp \
           splashscreen.cpp \
	   qgsacetateobject.cpp \
           qgslabeldialog.cpp \
           qgslabel.cpp \
           qgslabelattributes.cpp \
	   qgsacetaterectangle.cpp \
     qgsuvaldialog.cpp \
     qgsuniquevalrenderer.cpp
