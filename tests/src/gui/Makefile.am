# Copyright (C) 2003 Gary Sherman <sherman at mrcc.com>
#  
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without 
# modifications, as long as this notice is preserved.
# 
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

bin_PROGRAMS = testqgisapp \
		testqgsabout \
		testqgsaddattrdialog \
		testqgsattributeactiondialog \
		testqgsattributedialog \
		testqgsattributetable \
		testqgsattributetabledisplay \
		testqgsbookmarks \
		testqgsclipboard \
		testqgscontinuouscolordialog \
		testqgscontinuouscolorrenderer \
		testqgscoordinatetransform \
		testqgscustomprojectiondialog \
		testqgsdelattrdialog \
		testqgsencodingfiledialog \
		testqgsfillstylewidget \
		testqgsgeomtypedialog \
		testqgsgraduatedsymboldialog \
		testqgsgraduatedsymbolrenderer \
		testqgshelpviewer \
		testqgsidentifyresults \
		testqgslabeldialog \
		testqgslayerprojectionselector \
		testqgslinestyledialog \
		testqgslinestylewidget \
		testqgsludialog \
		testqgsmapcanvas \
		testqgsmapcanvasitem \
		testqgsmapcanvasmap \
		testqgsmaplayer \
		testqgsmaplayerregistry \
		testqgsmapoverviewcanvas \
		testqgsmaprender \
		testqgsmapserverexport \
		testqgsmaptool \
		testqgsmaptoolcapture \
		testqgsmaptoolidentify \
		testqgsmaptoolpan \
		testqgsmaptoolselect \
		testqgsmaptoolvertexedit \
		testqgsmaptoolzoom \
		testqgsmarkerdialog \
		testqgsmeasure \
		testqgsmessageviewer \
		testqgsnewconnection \
		testqgsnewhttpconnection \
		testqgsoptions \
		testqgspastetransformations \
		testqgspatterndialog \
		testqgspgquerybuilder \
		testqgspgutil \
		testqgspluginmanager \
		testqgspluginmetadata \
		testqgspointstylewidget \
		testqgsproject \
		testqgsprojectproperties \
		testqgsrasterlayerproperties \
		testqgsrubberband \
		testqgsrunprocess \
		testqgssearchquerybuilder \
		testqgsserversourceselect \
		testqgssinglesymboldialog \
		testqgssinglesymbolrenderer \
		testqgsspatialrefsys \
		testqgsuniquevaluedialog \
		testqgsuniquevaluerenderer \
		testqgsvectorfilewriter \
		testqgsvectorlayer \
		testqgsvectorlayerproperties \
		testqgsvertexmarker 

#		testqgsdbsourceselect 
#		testqgslabel 
#		testqgsvectorsymbologywidget 

#
# Define some global variables that will be used for building each test
#

GLOBALLDADD = $(QT_LDADD) \
				$(PG_LIB) \
				$(GDAL_LDADD) \
				-lproj \
				../../../src/core/libqgis_core.la \
				../../../src/raster/libqgis_raster.la \
				../../../src/legend/libqgis_legend.la \
				../../../src/gui/libqgis_gui.la  
GLOBALCXXFLAGS = $(CXXFLAGS) \
				$(EXTRA_CXXFLAGS) \
				$(GDAL_CFLAGS) \
				$(QT_CXXFLAGS) \
				$(PG_INC) \
				-I../../../src/raster \
				-I../../../src/core \
				-I../../../src/widgets/projectionselector \
				-I../../../src/ui \
				-I../../../src/gui

#
# Instruction for running the qt4 meta object compiler 
#

%.moc.cpp: %.cpp
	$(MOC) -o $@ $<

BUILT_SOURCES = $(testqgisapp_MOC) \
		$(testqgsvertexmarker_MOC) \
		$(testqgsvectorsymbologywidget_MOC) \
		$(testqgsvectorlayerproperties_MOC) \
		$(testqgsvectorlayer_MOC) \
		$(testqgsvectorfilewriter_MOC) \
		$(testqgsuniquevaluerenderer_MOC) \
		$(testqgsuniquevaluedialog_MOC) \
		$(testqgsspatialrefsys_MOC) \
		$(testqgssinglesymbolrenderer_MOC) \
		$(testqgssinglesymboldialog_MOC) \
		$(testqgsserversourceselect_MOC) \
		$(testqgssearchquerybuilder_MOC) \
		$(testqgsrunprocess_MOC) \
		$(testqgsrubberband_MOC) \
		$(testqgsrasterlayerproperties_MOC) \
		$(testqgsprojectproperties_MOC) \
		$(testqgsproject_MOC) \
		$(testqgspointstylewidget_MOC) \
		$(testqgspluginmetadata_MOC) \
		$(testqgspluginmanager_MOC) \
		$(testqgspgutil_MOC) \
		$(testqgspgquerybuilder_MOC) \
		$(testqgspatterndialog_MOC) \
		$(testqgspastetransformations_MOC) \
		$(testqgsoptions_MOC) \
		$(testqgsnewhttpconnection_MOC) \
		$(testqgsnewconnection_MOC) \
		$(testqgsmessageviewer_MOC) \
		$(testqgsmeasure_MOC) \
		$(testqgsmarkerdialog_MOC) \
		$(testqgsmaptoolzoom_MOC) \
		$(testqgsmaptoolvertexedit_MOC) \
		$(testqgsmaptoolselect_MOC) \
		$(testqgsmaptoolpan_MOC) \
		$(testqgsmaptoolidentify_MOC) \
		$(testqgsmaptool_MOC) \
		$(testqgsmaptoolcapture_MOC) \
		$(testqgsmapserverexport_MOC) \
		$(testqgsmaprender_MOC) \
		$(testqgsmapoverviewcanvas_MOC) \
		$(testqgsmaplayerregistry_MOC) \
		$(testqgsmaplayer_MOC) \
		$(testqgsmapcanvasmap_MOC) \
		$(testqgsmapcanvasitem_MOC) \
		$(testqgsmapcanvas_MOC) \
		$(testqgsludialog_MOC) \
		$(testqgslinestylewidget_MOC) \
		$(testqgslinestyledialog_MOC) \
		$(testqgslayerprojectionselector_MOC) \
		$(testqgslabeldialog_MOC) \
		$(testqgslabel_MOC) \
		$(testqgsidentifyresults_MOC) \
		$(testqgshelpviewer_MOC) \
		$(testqgsgraduatedsymbolrenderer_MOC) \
		$(testqgsgraduatedsymboldialog_MOC) \
		$(testqgsgeomtypedialog_MOC) \
		$(testqgsfillstylewidget_MOC) \
		$(testqgsencodingfiledialog_MOC) \
		$(testqgsdelattrdialog_MOC) \
		$(testqgsdbsourceselect_MOC) \
		$(testqgscustomprojectiondialog_MOC) \
		$(testqgscoordinatetransform_MOC) \
		$(testqgscontinuouscolorrenderer_MOC) \
		$(testqgscontinuouscolordialog_MOC) \
		$(testqgsclipboard_MOC) \
		$(testqgsbookmarks_MOC) \
		$(testqgsattributetabledisplay_MOC) \
		$(testqgsattributetable_MOC) \
		$(testqgsattributedialog_MOC) \
		$(testqgsattributeactiondialog_MOC) \
		$(testqgsaddattrdialog_MOC) \
		$(testqgsabout_MOC) 

CLEANFILES = $(BUILT_SOURCES)

#
# Specify the compilation files for each unit test now
#

testqgisapp_MOC = testqgisapp.moc.cpp
testqgisapp_SOURCES = testqgisapp.cpp
testqgisapp_LDADD = $(GLOBALLDADD)
testqgisapp_CXXFLAGS =  $(GLOBALCXXFLAGS)

testqgsabout_MOC = testqgsabout.moc.cpp
testqgsabout_SOURCES = testqgsabout.cpp
testqgsabout_LDADD = $(GLOBALLDADD)
testqgsabout_CXXFLAGS =  $(GLOBALCXXFLAGS)

testqgsaddattrdialog_MOC = testqgsaddattrdialog.moc.cpp
testqgsaddattrdialog_SOURCES = testqgsaddattrdialog.cpp
testqgsaddattrdialog_LDADD = $(GLOBALLDADD)
testqgsaddattrdialog_CXXFLAGS =  $(GLOBALCXXFLAGS)

testqgsattributeactiondialog_MOC = testqgsattributeactiondialog.moc.cpp
testqgsattributeactiondialog_SOURCES = testqgsattributeactiondialog.cpp
testqgsattributeactiondialog_LDADD = $(GLOBALLDADD)
testqgsattributeactiondialog_CXXFLAGS =  $(GLOBALCXXFLAGS)

testqgsattributedialog_MOC = testqgsattributedialog.moc.cpp
testqgsattributedialog_SOURCES = testqgsattributedialog.cpp
testqgsattributedialog_LDADD = $(GLOBALLDADD)
testqgsattributedialog_CXXFLAGS =  $(GLOBALCXXFLAGS)

testqgsattributetable_MOC = testqgsattributetable.moc.cpp
testqgsattributetable_SOURCES = testqgsattributetable.cpp
testqgsattributetable_LDADD = $(GLOBALLDADD)
testqgsattributetable_CXXFLAGS =  $(GLOBALCXXFLAGS)

testqgsattributetabledisplay_MOC = testqgsattributetabledisplay.moc.cpp
testqgsattributetabledisplay_SOURCES = testqgsattributetabledisplay.cpp
testqgsattributetabledisplay_LDADD = $(GLOBALLDADD)
testqgsattributetabledisplay_CXXFLAGS =  $(GLOBALCXXFLAGS)

testqgsbookmarks_MOC = testqgsbookmarks.moc.cpp
testqgsbookmarks_SOURCES = testqgsbookmarks.cpp
testqgsbookmarks_LDADD = $(GLOBALLDADD)
testqgsbookmarks_CXXFLAGS =  $(GLOBALCXXFLAGS)

testqgsclipboard_MOC = testqgsclipboard.moc.cpp
testqgsclipboard_SOURCES = testqgsclipboard.cpp
testqgsclipboard_LDADD = $(GLOBALLDADD)
testqgsclipboard_CXXFLAGS =  $(GLOBALCXXFLAGS)

testqgscontinuouscolordialog_MOC = testqgscontinuouscolordialog.moc.cpp
testqgscontinuouscolordialog_SOURCES = testqgscontinuouscolordialog.cpp
testqgscontinuouscolordialog_LDADD = $(GLOBALLDADD)
testqgscontinuouscolordialog_CXXFLAGS =  $(GLOBALCXXFLAGS)

testqgscontinuouscolorrenderer_MOC = testqgscontinuouscolorrenderer.moc.cpp
testqgscontinuouscolorrenderer_SOURCES = testqgscontinuouscolorrenderer.cpp
testqgscontinuouscolorrenderer_LDADD = $(GLOBALLDADD)
testqgscontinuouscolorrenderer_CXXFLAGS =  $(GLOBALCXXFLAGS)

testqgscoordinatetransform_MOC = testqgscoordinatetransform.moc.cpp
testqgscoordinatetransform_SOURCES = testqgscoordinatetransform.cpp
testqgscoordinatetransform_LDADD = $(GLOBALLDADD)
testqgscoordinatetransform_CXXFLAGS =  $(GLOBALCXXFLAGS)

testqgscustomprojectiondialog_MOC = testqgscustomprojectiondialog.moc.cpp
testqgscustomprojectiondialog_SOURCES = testqgscustomprojectiondialog.cpp
testqgscustomprojectiondialog_LDADD = $(GLOBALLDADD)
testqgscustomprojectiondialog_CXXFLAGS =  $(GLOBALCXXFLAGS)

testqgsdbsourceselect_MOC = testqgsdbsourceselect.moc.cpp
testqgsdbsourceselect_SOURCES = testqgsdbsourceselect.cpp
testqgsdbsourceselect_LDADD = $(GLOBALLDADD)
testqgsdbsourceselect_CXXFLAGS =  $(GLOBALCXXFLAGS)

testqgsdelattrdialog_MOC = testqgsdelattrdialog.moc.cpp
testqgsdelattrdialog_SOURCES = testqgsdelattrdialog.cpp
testqgsdelattrdialog_LDADD = $(GLOBALLDADD)
testqgsdelattrdialog_CXXFLAGS =  $(GLOBALCXXFLAGS)

testqgsencodingfiledialog_MOC = testqgsencodingfiledialog.moc.cpp
testqgsencodingfiledialog_SOURCES = testqgsencodingfiledialog.cpp
testqgsencodingfiledialog_LDADD = $(GLOBALLDADD)
testqgsencodingfiledialog_CXXFLAGS =  $(GLOBALCXXFLAGS)

testqgsfillstylewidget_MOC = testqgsfillstylewidget.moc.cpp
testqgsfillstylewidget_SOURCES = testqgsfillstylewidget.cpp
testqgsfillstylewidget_LDADD = $(GLOBALLDADD)
testqgsfillstylewidget_CXXFLAGS =  $(GLOBALCXXFLAGS)

testqgsgeomtypedialog_MOC = testqgsgeomtypedialog.moc.cpp
testqgsgeomtypedialog_SOURCES = testqgsgeomtypedialog.cpp
testqgsgeomtypedialog_LDADD = $(GLOBALLDADD)
testqgsgeomtypedialog_CXXFLAGS =  $(GLOBALCXXFLAGS)

testqgsgraduatedsymboldialog_MOC = testqgsgraduatedsymboldialog.moc.cpp
testqgsgraduatedsymboldialog_SOURCES = testqgsgraduatedsymboldialog.cpp
testqgsgraduatedsymboldialog_LDADD = $(GLOBALLDADD)
testqgsgraduatedsymboldialog_CXXFLAGS =  $(GLOBALCXXFLAGS)

testqgsgraduatedsymbolrenderer_MOC = testqgsgraduatedsymbolrenderer.moc.cpp
testqgsgraduatedsymbolrenderer_SOURCES = testqgsgraduatedsymbolrenderer.cpp
testqgsgraduatedsymbolrenderer_LDADD = $(GLOBALLDADD)
testqgsgraduatedsymbolrenderer_CXXFLAGS =  $(GLOBALCXXFLAGS)

testqgshelpviewer_MOC = testqgshelpviewer.moc.cpp
testqgshelpviewer_SOURCES = testqgshelpviewer.cpp
testqgshelpviewer_LDADD = $(GLOBALLDADD)
testqgshelpviewer_CXXFLAGS =  $(GLOBALCXXFLAGS)

testqgsidentifyresults_MOC = testqgsidentifyresults.moc.cpp
testqgsidentifyresults_SOURCES = testqgsidentifyresults.cpp
testqgsidentifyresults_LDADD = $(GLOBALLDADD)
testqgsidentifyresults_CXXFLAGS =  $(GLOBALCXXFLAGS)

testqgslabel_MOC = testqgslabel.moc.cpp
testqgslabel_SOURCES = testqgslabel.cpp
testqgslabel_LDADD = $(GLOBALLDADD)
testqgslabel_CXXFLAGS =  $(GLOBALCXXFLAGS)

testqgslabeldialog_MOC = testqgslabeldialog.moc.cpp
testqgslabeldialog_SOURCES = testqgslabeldialog.cpp
testqgslabeldialog_LDADD = $(GLOBALLDADD)
testqgslabeldialog_CXXFLAGS =  $(GLOBALCXXFLAGS)

testqgslayerprojectionselector_MOC = testqgslayerprojectionselector.moc.cpp
testqgslayerprojectionselector_SOURCES = testqgslayerprojectionselector.cpp
testqgslayerprojectionselector_LDADD = $(GLOBALLDADD)
testqgslayerprojectionselector_CXXFLAGS =  $(GLOBALCXXFLAGS)

testqgslinestyledialog_MOC = testqgslinestyledialog.moc.cpp
testqgslinestyledialog_SOURCES = testqgslinestyledialog.cpp
testqgslinestyledialog_LDADD = $(GLOBALLDADD)
testqgslinestyledialog_CXXFLAGS =  $(GLOBALCXXFLAGS)

testqgslinestylewidget_MOC = testqgslinestylewidget.moc.cpp
testqgslinestylewidget_SOURCES = testqgslinestylewidget.cpp
testqgslinestylewidget_LDADD = $(GLOBALLDADD)
testqgslinestylewidget_CXXFLAGS =  $(GLOBALCXXFLAGS)

testqgsludialog_MOC = testqgsludialog.moc.cpp
testqgsludialog_SOURCES = testqgsludialog.cpp
testqgsludialog_LDADD = $(GLOBALLDADD)
testqgsludialog_CXXFLAGS =  $(GLOBALCXXFLAGS)

testqgsmapcanvas_MOC = testqgsmapcanvas.moc.cpp
testqgsmapcanvas_SOURCES = testqgsmapcanvas.cpp
testqgsmapcanvas_LDADD = $(GLOBALLDADD)
testqgsmapcanvas_CXXFLAGS =  $(GLOBALCXXFLAGS)

testqgsmapcanvasitem_MOC = testqgsmapcanvasitem.moc.cpp
testqgsmapcanvasitem_SOURCES = testqgsmapcanvasitem.cpp
testqgsmapcanvasitem_LDADD = $(GLOBALLDADD)
testqgsmapcanvasitem_CXXFLAGS =  $(GLOBALCXXFLAGS)

testqgsmapcanvasmap_MOC = testqgsmapcanvasmap.moc.cpp
testqgsmapcanvasmap_SOURCES = testqgsmapcanvasmap.cpp
testqgsmapcanvasmap_LDADD = $(GLOBALLDADD)
testqgsmapcanvasmap_CXXFLAGS =  $(GLOBALCXXFLAGS)

testqgsmaplayer_MOC = testqgsmaplayer.moc.cpp
testqgsmaplayer_SOURCES = testqgsmaplayer.cpp
testqgsmaplayer_LDADD = $(GLOBALLDADD)
testqgsmaplayer_CXXFLAGS =  $(GLOBALCXXFLAGS)

testqgsmaplayerregistry_MOC = testqgsmaplayerregistry.moc.cpp
testqgsmaplayerregistry_SOURCES = testqgsmaplayerregistry.cpp
testqgsmaplayerregistry_LDADD = $(GLOBALLDADD)
testqgsmaplayerregistry_CXXFLAGS =  $(GLOBALCXXFLAGS)

testqgsmapoverviewcanvas_MOC = testqgsmapoverviewcanvas.moc.cpp
testqgsmapoverviewcanvas_SOURCES = testqgsmapoverviewcanvas.cpp
testqgsmapoverviewcanvas_LDADD = $(GLOBALLDADD)
testqgsmapoverviewcanvas_CXXFLAGS =  $(GLOBALCXXFLAGS)

testqgsmaprender_MOC = testqgsmaprender.moc.cpp
testqgsmaprender_SOURCES = testqgsmaprender.cpp
testqgsmaprender_LDADD = $(GLOBALLDADD)
testqgsmaprender_CXXFLAGS =  $(GLOBALCXXFLAGS)

testqgsmapserverexport_MOC = testqgsmapserverexport.moc.cpp
testqgsmapserverexport_SOURCES = testqgsmapserverexport.cpp
testqgsmapserverexport_LDADD = $(GLOBALLDADD)
testqgsmapserverexport_CXXFLAGS =  $(GLOBALCXXFLAGS)

testqgsmaptoolcapture_MOC = testqgsmaptoolcapture.moc.cpp
testqgsmaptoolcapture_SOURCES = testqgsmaptoolcapture.cpp
testqgsmaptoolcapture_LDADD = $(GLOBALLDADD)
testqgsmaptoolcapture_CXXFLAGS =  $(GLOBALCXXFLAGS)

testqgsmaptool_MOC = testqgsmaptool.moc.cpp
testqgsmaptool_SOURCES = testqgsmaptool.cpp
testqgsmaptool_LDADD = $(GLOBALLDADD)
testqgsmaptool_CXXFLAGS =  $(GLOBALCXXFLAGS)

testqgsmaptoolidentify_MOC = testqgsmaptoolidentify.moc.cpp
testqgsmaptoolidentify_SOURCES = testqgsmaptoolidentify.cpp
testqgsmaptoolidentify_LDADD = $(GLOBALLDADD)
testqgsmaptoolidentify_CXXFLAGS =  $(GLOBALCXXFLAGS)

testqgsmaptoolpan_MOC = testqgsmaptoolpan.moc.cpp
testqgsmaptoolpan_SOURCES = testqgsmaptoolpan.cpp
testqgsmaptoolpan_LDADD = $(GLOBALLDADD)
testqgsmaptoolpan_CXXFLAGS =  $(GLOBALCXXFLAGS)

testqgsmaptoolselect_MOC = testqgsmaptoolselect.moc.cpp
testqgsmaptoolselect_SOURCES = testqgsmaptoolselect.cpp
testqgsmaptoolselect_LDADD = $(GLOBALLDADD)
testqgsmaptoolselect_CXXFLAGS =  $(GLOBALCXXFLAGS)

testqgsmaptoolvertexedit_MOC = testqgsmaptoolvertexedit.moc.cpp
testqgsmaptoolvertexedit_SOURCES = testqgsmaptoolvertexedit.cpp
testqgsmaptoolvertexedit_LDADD = $(GLOBALLDADD)
testqgsmaptoolvertexedit_CXXFLAGS =  $(GLOBALCXXFLAGS)

testqgsmaptoolzoom_MOC = testqgsmaptoolzoom.moc.cpp
testqgsmaptoolzoom_SOURCES = testqgsmaptoolzoom.cpp
testqgsmaptoolzoom_LDADD = $(GLOBALLDADD)
testqgsmaptoolzoom_CXXFLAGS =  $(GLOBALCXXFLAGS)

testqgsmarkerdialog_MOC = testqgsmarkerdialog.moc.cpp
testqgsmarkerdialog_SOURCES = testqgsmarkerdialog.cpp
testqgsmarkerdialog_LDADD = $(GLOBALLDADD)
testqgsmarkerdialog_CXXFLAGS =  $(GLOBALCXXFLAGS)

testqgsmeasure_MOC = testqgsmeasure.moc.cpp
testqgsmeasure_SOURCES = testqgsmeasure.cpp
testqgsmeasure_LDADD = $(GLOBALLDADD)
testqgsmeasure_CXXFLAGS =  $(GLOBALCXXFLAGS)

testqgsmessageviewer_MOC = testqgsmessageviewer.moc.cpp
testqgsmessageviewer_SOURCES = testqgsmessageviewer.cpp
testqgsmessageviewer_LDADD = $(GLOBALLDADD)
testqgsmessageviewer_CXXFLAGS =  $(GLOBALCXXFLAGS)

testqgsnewconnection_MOC = testqgsnewconnection.moc.cpp
testqgsnewconnection_SOURCES = testqgsnewconnection.cpp
testqgsnewconnection_LDADD = $(GLOBALLDADD)
testqgsnewconnection_CXXFLAGS =  $(GLOBALCXXFLAGS)

testqgsnewhttpconnection_MOC = testqgsnewhttpconnection.moc.cpp
testqgsnewhttpconnection_SOURCES = testqgsnewhttpconnection.cpp
testqgsnewhttpconnection_LDADD = $(GLOBALLDADD)
testqgsnewhttpconnection_CXXFLAGS =  $(GLOBALCXXFLAGS)

testqgsoptions_MOC = testqgsoptions.moc.cpp
testqgsoptions_SOURCES = testqgsoptions.cpp
testqgsoptions_LDADD = $(GLOBALLDADD)
testqgsoptions_CXXFLAGS =  $(GLOBALCXXFLAGS)

testqgspastetransformations_MOC = testqgspastetransformations.moc.cpp
testqgspastetransformations_SOURCES = testqgspastetransformations.cpp
testqgspastetransformations_LDADD = $(GLOBALLDADD)
testqgspastetransformations_CXXFLAGS =  $(GLOBALCXXFLAGS)

testqgspatterndialog_MOC = testqgspatterndialog.moc.cpp
testqgspatterndialog_SOURCES = testqgspatterndialog.cpp
testqgspatterndialog_LDADD = $(GLOBALLDADD)
testqgspatterndialog_CXXFLAGS =  $(GLOBALCXXFLAGS)

testqgspgquerybuilder_MOC = testqgspgquerybuilder.moc.cpp
testqgspgquerybuilder_SOURCES = testqgspgquerybuilder.cpp
testqgspgquerybuilder_LDADD = $(GLOBALLDADD)
testqgspgquerybuilder_CXXFLAGS =  $(GLOBALCXXFLAGS)

testqgspgutil_MOC = testqgspgutil.moc.cpp
testqgspgutil_SOURCES = testqgspgutil.cpp
testqgspgutil_LDADD = $(GLOBALLDADD)
testqgspgutil_CXXFLAGS =  $(GLOBALCXXFLAGS)

testqgspluginmanager_MOC = testqgspluginmanager.moc.cpp
testqgspluginmanager_SOURCES = testqgspluginmanager.cpp
testqgspluginmanager_LDADD = $(GLOBALLDADD)
testqgspluginmanager_CXXFLAGS =  $(GLOBALCXXFLAGS)

testqgspluginmetadata_MOC = testqgspluginmetadata.moc.cpp
testqgspluginmetadata_SOURCES = testqgspluginmetadata.cpp
testqgspluginmetadata_LDADD = $(GLOBALLDADD)
testqgspluginmetadata_CXXFLAGS =  $(GLOBALCXXFLAGS)

testqgspointstylewidget_MOC = testqgspointstylewidget.moc.cpp
testqgspointstylewidget_SOURCES = testqgspointstylewidget.cpp
testqgspointstylewidget_LDADD = $(GLOBALLDADD)
testqgspointstylewidget_CXXFLAGS =  $(GLOBALCXXFLAGS)

testqgsproject_MOC = testqgsproject.moc.cpp
testqgsproject_SOURCES = testqgsproject.cpp
testqgsproject_LDADD = $(GLOBALLDADD)
testqgsproject_CXXFLAGS =  $(GLOBALCXXFLAGS)

testqgsprojectproperties_MOC = testqgsprojectproperties.moc.cpp
testqgsprojectproperties_SOURCES = testqgsprojectproperties.cpp
testqgsprojectproperties_LDADD = $(GLOBALLDADD)
testqgsprojectproperties_CXXFLAGS =  $(GLOBALCXXFLAGS)

testqgsrasterlayerproperties_MOC = testqgsrasterlayerproperties.moc.cpp
testqgsrasterlayerproperties_SOURCES = testqgsrasterlayerproperties.cpp
testqgsrasterlayerproperties_LDADD = $(GLOBALLDADD)
testqgsrasterlayerproperties_CXXFLAGS =  $(GLOBALCXXFLAGS)

testqgsrubberband_MOC = testqgsrubberband.moc.cpp
testqgsrubberband_SOURCES = testqgsrubberband.cpp
testqgsrubberband_LDADD = $(GLOBALLDADD)
testqgsrubberband_CXXFLAGS =  $(GLOBALCXXFLAGS)

testqgsrunprocess_MOC = testqgsrunprocess.moc.cpp
testqgsrunprocess_SOURCES = testqgsrunprocess.cpp
testqgsrunprocess_LDADD = $(GLOBALLDADD)
testqgsrunprocess_CXXFLAGS =  $(GLOBALCXXFLAGS)

testqgssearchquerybuilder_MOC = testqgssearchquerybuilder.moc.cpp
testqgssearchquerybuilder_SOURCES = testqgssearchquerybuilder.cpp
testqgssearchquerybuilder_LDADD = $(GLOBALLDADD)
testqgssearchquerybuilder_CXXFLAGS =  $(GLOBALCXXFLAGS)

testqgsserversourceselect_MOC = testqgsserversourceselect.moc.cpp
testqgsserversourceselect_SOURCES = testqgsserversourceselect.cpp
testqgsserversourceselect_LDADD = $(GLOBALLDADD)
testqgsserversourceselect_CXXFLAGS =  $(GLOBALCXXFLAGS)

testqgssinglesymboldialog_MOC = testqgssinglesymboldialog.moc.cpp
testqgssinglesymboldialog_SOURCES = testqgssinglesymboldialog.cpp
testqgssinglesymboldialog_LDADD = $(GLOBALLDADD)
testqgssinglesymboldialog_CXXFLAGS =  $(GLOBALCXXFLAGS)

testqgssinglesymbolrenderer_MOC = testqgssinglesymbolrenderer.moc.cpp
testqgssinglesymbolrenderer_SOURCES = testqgssinglesymbolrenderer.cpp
testqgssinglesymbolrenderer_LDADD = $(GLOBALLDADD)
testqgssinglesymbolrenderer_CXXFLAGS =  $(GLOBALCXXFLAGS)

testqgsspatialrefsys_MOC = testqgsspatialrefsys.moc.cpp
testqgsspatialrefsys_SOURCES = testqgsspatialrefsys.cpp
testqgsspatialrefsys_LDADD = $(GLOBALLDADD)
testqgsspatialrefsys_CXXFLAGS =  $(GLOBALCXXFLAGS)

testqgsuniquevaluedialog_MOC = testqgsuniquevaluedialog.moc.cpp
testqgsuniquevaluedialog_SOURCES = testqgsuniquevaluedialog.cpp
testqgsuniquevaluedialog_LDADD = $(GLOBALLDADD)
testqgsuniquevaluedialog_CXXFLAGS =  $(GLOBALCXXFLAGS)

testqgsuniquevaluerenderer_MOC = testqgsuniquevaluerenderer.moc.cpp
testqgsuniquevaluerenderer_SOURCES = testqgsuniquevaluerenderer.cpp
testqgsuniquevaluerenderer_LDADD = $(GLOBALLDADD)
testqgsuniquevaluerenderer_CXXFLAGS =  $(GLOBALCXXFLAGS)

testqgsvectorfilewriter_MOC = testqgsvectorfilewriter.moc.cpp
testqgsvectorfilewriter_SOURCES = testqgsvectorfilewriter.cpp
testqgsvectorfilewriter_LDADD = $(GLOBALLDADD)
testqgsvectorfilewriter_CXXFLAGS =  $(GLOBALCXXFLAGS)

testqgsvectorlayer_MOC = testqgsvectorlayer.moc.cpp
testqgsvectorlayer_SOURCES = testqgsvectorlayer.cpp
testqgsvectorlayer_LDADD = $(GLOBALLDADD)
testqgsvectorlayer_CXXFLAGS =  $(GLOBALCXXFLAGS)

testqgsvectorlayerproperties_MOC = testqgsvectorlayerproperties.moc.cpp
testqgsvectorlayerproperties_SOURCES = testqgsvectorlayerproperties.cpp
testqgsvectorlayerproperties_LDADD = $(GLOBALLDADD)
testqgsvectorlayerproperties_CXXFLAGS =  $(GLOBALCXXFLAGS)

testqgsvectorsymbologywidget_MOC = testqgsvectorsymbologywidget.moc.cpp
testqgsvectorsymbologywidget_SOURCES = testqgsvectorsymbologywidget.cpp
testqgsvectorsymbologywidget_LDADD = $(GLOBALLDADD)
testqgsvectorsymbologywidget_CXXFLAGS =  $(GLOBALCXXFLAGS)

testqgsvertexmarker_MOC = testqgsvertexmarker.moc.cpp
testqgsvertexmarker_SOURCES = testqgsvertexmarker.cpp
testqgsvertexmarker_LDADD = $(GLOBALLDADD)
testqgsvertexmarker_CXXFLAGS =  $(GLOBALCXXFLAGS)
