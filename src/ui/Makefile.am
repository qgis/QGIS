# Copyright (C) 2005 Tim Sutton tim@linfiniti.com
#  
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without 
# modifications, as long as this notice is preserved.
# 
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

ui_%.h: %.ui 
	$(UIC) -o $@ $<
	
WIDGETPATH=../../widgets
	
## qt designer files for all the GUI stuff

qgis_ui_UIC =	\
		qgsabout.ui				\
		qgsaddattrdialogbase.ui			\
		qgsadvancedattrsearchbase.ui		\
		qgsattributedialogbase.ui		\
		qgsattributeactiondialogbase.ui		\
		qgsattributetablebase.ui		\
		qgsbookmarksbase.ui		\
		qgscontcoldialogbase.ui			\
		qgsdelattrdialogbase.ui			\
		qgsvectorlayerpropertiesbase.ui	\
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
		qgscustomprojectiondialogbase.ui \
		qgscomposerbase.ui \
		qgscompositionbase.ui \
		qgsgeomtypedialogbase.ui \
		qgspastetransformationsbase.ui \
		qgscomposermapbase.ui \
		qgscomposerlabelbase.ui \
		qgscomposerpicturebase.ui \
		qgscomposerscalebarbase.ui \
		qgscomposervectorlegendbase.ui \
		qgscontcoldialogbase.ui \
		qgsattributedialogbase.ui \
		qgsfillstylewidgetbase.ui \
		qgsgrasydialogbase.ui \
		qgslabeldialogbase.ui \
		qgslinestylewidgetbase.ui \
		qgsmeasurebase.ui \
		qgspointstylewidgetbase.ui \
		qgsidentifyresultsbase.ui \
		qgsuvaldialogbase.ui \
		qgsattributetabledisplaybase.ui \
		qgsaddattrdialogbase.ui \
		qgsadvancedattrsearch.ui \
		qgsattributeactiondialogbase.ui \
		qgsdelattrdialogbase.ui \
		qgspgquerybuilderbase.ui \
		qgsludialogbase.ui \
		qgspatterndialogbase.ui \
		qgslinestyledialogbase.ui \
		qgsmarkerdialogbase.ui \
		qgsnewhttpconnectionbase.ui \
		qgsnewconnectionbase.ui \
		../widgets/projectionselector/qgsprojectionselectorbase.ui

## all the ui files generate a corresponding header file
qgis_ui_UIHEADERS = ui_qgisappbase.h \
		ui_qgsabout.h				\
		ui_qgsaddattrdialogbase.h			\
		ui_qgsadvancedattrsearchbase.h		\
		ui_qgsattributedialogbase.h		\
		ui_qgsattributeactiondialogbase.h		\
		ui_qgsattributetablebase.h		\
		ui_qgsbookmarksbase.h		\
		ui_qgscontcoldialogbase.h			\
		ui_qgsdelattrdialogbase.h			\
		ui_qgsvectorlayerpropertiesbase.h	\
		ui_qgsgeomtypedialogbase.h		\
		ui_qgsgrasydialogbase.h			\
		ui_qgshelpviewerbase.h			\
		ui_qgsidentifyresultsbase.h		\
		ui_qgslabeldialogbase.h			\
		ui_qgslinestyledialogbase.h		\
		ui_qgsludialogbase.h			\
		ui_qgsmapserverexportbase.h		\
		ui_qgsmarkerdialogbase.h			\
		ui_qgsmessageviewer.h			\
		ui_qgsmeasurebase.h			\
		ui_qgsnewhttpconnectionbase.h		\
		ui_qgsoptionsbase.h			\
		ui_qgspastetransformationsbase.h		\
		ui_qgspatterndialogbase.h			\
		ui_qgspluginmanagerbase.h			\
		ui_qgsprojectpropertiesbase.h		\
		ui_qgsrasterlayerpropertiesbase.h		\
		ui_qgsserversourceselectbase.h		\
		ui_qgssisydialogbase.h			\
		ui_qgsuvaldialogbase.h			\
		ui_qgslayerprojectionselectorbase.h \
		ui_qgscustomprojectiondialogbase.h \
		ui_qgscomposerbase.h \
		ui_qgscompositionbase.h \
		ui_qgsgeomtypedialogbase.h \
		ui_qgspastetransformationsbase.h \
		ui_qgscomposermapbase.h \
		ui_qgscomposerlabelbase.h \
		ui_qgscomposerpicturebase.h \
		ui_qgscomposerscalebarbase.h \
		ui_qgscomposervectorlegendbase.h \
		ui_qgscontcoldialogbase.h \
		ui_qgsattributedialogbase.h \
		ui_qgsfillstylewidgetbase.h \
		ui_qgsgrasydialogbase.h \
		ui_qgslabeldialogbase.h \
		ui_qgslinestylewidgetbase.h \
		ui_qgsmeasurebase.h \
		ui_qgspointstylewidgetbase.h \
		ui_qgsidentifyresultsbase.h \
		ui_qgsaddattrdialogbase.h \
		ui_qgsadvancedattrsearchbase.h \
		ui_qgsattributeactiondialogbase.h \
		ui_qgsdelattrdialogbase.h \
		ui_qgspgquerybuilderbase.h \
		ui_qgsludialogbase.h \
		ui_qgspatterndialogbase.h \
		ui_qgslinestyledialogbase.h \
		ui_qgsmarkerdialogbase.h \
		ui_qgsuvaldialogbase.h \
		ui_qgsnewhttpconnectionbase.h \
		ui_qgsnewconnectionbase.h \
		../widgets/projectionselector/ui_qgsprojectionselectorbase.h


postgresUIHEADERS = ui_qgsdbsourceselectbase.h	\
		  ui_qgsnewconnectionbase.h		\
		  ui_qgspgquerybuilderbase.h	

BUILT_SOURCES = $(qgis_ui_UIHEADERS) $(postgresUIHEADERS) 

qgis_ui_LDADD = $(LDADD) $(QT_LDADD) 
qgis_ui_CXXFLAGS = $(PREFIX) $(PLUGINPATH) $(PKGDATAPATH) $(GDAL_CFLAGS) $(CXXFLAGS) $(EXTRA_CXXFLAGS) $(QT_CXXFLAGS) $(PG_INC) $(DEBUG_QGIS) -I../widgets/projectionselector/ -I./legend/ -I./raster/

CLEANFILES = $(BUILT_SOURCES)

