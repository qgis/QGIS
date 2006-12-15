#################################################################
#
#         QMAKE Project File for Quantum GIS 
# 
#                   Tim Sutton 2006
#
# NOTE: Do not place any hard coded external paths in this file
#       all libs and includes should be specified in settings.pro
#       in the top level qgis directory.
# 
#################################################################

#
# This file builds the gui library - the app is built in a separate pro file
#

include(../../settings.pro)
include(../ui/ui.pro)
TEMPLATE=lib
TARGET=qgis_gui
#suffix debug to target if applicable
CONFIG(debug, debug|release){
  TARGET = $$member(TARGET, 0)-debug
}
LIBS += $${GDALLIBADD}
LIBS += $${POSTGRESLIBADD}
#LIBS += $${GEOSLIBADD}
#LIBS += $${PROJLIBADD}
LIBS += $${QGISCORELIBADD}
LIBS += $${QGISPROJECTIONSELECTORLIBADD}
message("QGIS Core Lib: $${QGISCORELIBADD}")
message("QGIS Projection Selector Lib: $${QGISPROJECTIONSELECTORLIBADD}")
message("QGIS Composer Lib: $${QGISCOMPOSERLIBADD}")
message("LIBS: $${LIBS}")
DESTDIR=$${QGISLIBDIR}
QT += qt3support svg core gui xml network
message("Building libs into $${DESTDIR}")


HEADERS =							\
                qgisapp.h                                       \
		qgisiface.h					\
		qgisinterface.h					\
		qgsabout.h					\
		qgsaddattrdialog.h				\
		qgsattributeaction.h				\
		qgsattributeactiondialog.h			\
		qgsattributedialog.h				\
		qgsattributetable.h				\
		qgsattributetabledisplay.h			\
		qgsbookmarks.h					\	
		qgsclipboard.h					\
		qgscolorbutton.h				\
		qgscontinuouscolordialog.h			\
		qgscontinuouscolorrenderer.h			\
		qgscsexception.h				\
		qgscursors.h					\
		qgscustomprojectiondialog.h			\
		qgsdelattrdialog.h				\
		qgsencodingfiledialog.h				\
		qgsgeomtypedialog.h				\
		qgsgraduatedsymboldialog.h			\
		qgsgraduatedsymbolrenderer.h			\
		qgshelpviewer.h					\
		qgsidentifyresults.h				\
		qgslabel.h					\
		qgslabeldialog.h				\
		qgslayerprojectionselector.h 			\
		qgslinestyledialog.h				\
		qgsludialog.h					\
		qgsmapcanvas.h					\
		qgsmapcanvasitem.h				\
		qgsmapcanvasmap.h				\
		qgsmaplayer.h					\
		qgsmaplayerinterface.h				\
		qgsmaplayerregistry.h				\
		qgsmaplayerset.h				\
		qgsmapoverviewcanvas.h				\
		qgsmaprender.h					\
		qgsmaptool.h					\
		qgsmaptoolcapture.h				\
		qgsmaptoolidentify.h				\
		qgsmaptoolpan.h					\
		qgsmaptoolselect.h				\
		qgsmaptoolvertexedit.h				\
		qgsmaptoolzoom.h				\
		qgsmarkerdialog.h				\
		qgsmeasure.h					\
		qgsmessageviewer.h				\
		qgsnewhttpconnection.h				\
		qgsoptions.h					\
		qgspastetransformations.h			\
		qgspatterndialog.h				\
		qgspluginmanager.h				\
		qgspluginregistry.h				\
		qgspluginmetadata.h				\
		qgsproject.h					\
		qgsprojectproperties.h				\
		qgsrasterlayerproperties.h			\
		qgsrubberband.h					\
		qgsrunprocess.h					\
		qgssearchquerybuilder.h				\
		qgsserversourceselect.h				\
		qgssinglesymboldialog.h				\
		qgssinglesymbolrenderer.h			\
		qgsuniquevaluedialog.h				\
		qgsuniquevaluerenderer.h			\
		qgsvectorfilewriter.h				\
		qgsvectorlayer.h			\
		qgsvectorlayerproperties.h			\
		qgsvertexmarker.h				\
                ../raster/qgsrasterbandstats.h  		\
	        ../raster/qgsrasterpyramid.h    	\
	        ../raster/qgsrasterlayer.h      	\
	        ../raster/qgsrasterviewport.h   	\
		../legend/qgslegend.h			\
		../legend/qgslegenditem.h 		\
		../legend/qgslegendlayer.h 		\
		../legend/qgslegendlayerfile.h 		\
		../legend/qgslegendlayerfilegroup.h 	\
		../legend/qgslegendpropertygroup.h 	\
		../legend/qgslegendpropertyitem.h 	\
		../legend/qgslegendsymbologygroup.h 	\
		../legend/qgslegendsymbologyitem.h 	\
		../legend/qgslegendvectorsymbologyitem.h \
                ../composer/qgscomposer.h \
		../composer/qgscomposeritem.h	\
		../composer/qgscomposerlabel.h	\
		../composer/qgscomposerpicture.h	\
		../composer/qgscomposermap.h	\
		../composer/qgscomposerscalebar.h	\
		../composer/qgscomposervectorlegend.h \
		../composer/qgscomposerview.h	\
		../composer/qgscomposition.h \					
                qgsdbsourceselect.h \	         
                qgsnewconnection.h \		 
                qgspgquerybuilder.h \           
                qgspgutil.h #from pg lib

SOURCES =					\
                qgisapp.cpp                     \ # needed to ensure ui gets build for qgsiface
	        qgsattributeaction.cpp		\
		qgisiface.cpp			\
		qgisinterface.cpp	        \
		qgsabout.cpp		        \
		qgsaddattrdialog.cpp		\
		qgsattributeactiondialog.cpp	\
		qgsattributedialog.cpp		\
		qgsattributetable.cpp		\
		qgsattributetabledisplay.cpp	\
		qgsbookmarks.cpp	        \
		qgsclipboard.cpp		\
		qgscolorbutton.cpp		\
		qgscontinuouscolordialog.cpp	\
		qgscontinuouscolorrenderer.cpp	\
		qgscursors.cpp		        \
		qgscustomprojectiondialog.cpp	\
		qgsdelattrdialog.cpp		\
		qgsencodingfiledialog.cpp	\
		qgsgeomtypedialog.cpp		\
		qgsgraduatedsymboldialog.cpp	\
		qgsgraduatedsymbolrenderer.cpp	\
		qgshelpviewer.cpp		\
		qgsidentifyresults.cpp		\
		qgslabel.cpp			\
		qgslabeldialog.cpp		\
		qgslayerprojectionselector.cpp  \
		qgslinestyledialog.cpp		\
		qgsludialog.cpp			\
		qgsmapcanvas.cpp		\
		qgsmapcanvasitem.cpp		\
		qgsmapcanvasmap.cpp		\
		qgsmaplayer.cpp			\
		qgsmaplayerregistry.cpp		\
		qgsmaplayerset.cpp		\
		qgsmapoverviewcanvas.cpp	\
		qgsmaprender.cpp		\
		qgsmaptool.cpp			\
		qgsmaptoolcapture.cpp		\
		qgsmaptoolidentify.cpp		\
		qgsmaptoolpan.cpp		\
		qgsmaptoolselect.cpp		\
		qgsmaptoolvertexedit.cpp	\
		qgsmaptoolzoom.cpp		\
		qgsmarkerdialog.cpp		\
		qgsmeasure.cpp			\
		qgsmessageviewer.cpp	        \
		qgsnewhttpconnection.cpp	\
		qgsoptions.cpp			\
		qgspastetransformations.cpp	\
		qgspatterndialog.cpp		\
		qgspluginregistry.cpp		\
		qgspluginmanager.cpp		\
		qgspluginmetadata.cpp		\
		qgsproject.cpp			\
		qgsprojectproperties.cpp	\
		qgsrasterlayerproperties.cpp	\
		qgsrubberband.cpp		\
		qgsrunprocess.cpp		\
		qgssearchquerybuilder.cpp       \
		qgsserversourceselect.cpp	\
		qgssinglesymboldialog.cpp	\
		qgssinglesymbolrenderer.cpp	\
		qgsuniquevaluedialog.cpp	\
		qgsuniquevaluerenderer.cpp	\
		qgsvectorfilewriter.cpp		\
		qgsvectorlayer.cpp              \
		qgsvectorlayerproperties.cpp	\
		qgsvertexmarker.cpp 		\
		../raster/qgsrasterlayer.cpp  	\
		../legend/qgslegendgroup.cpp 	\
		../legend/qgslegend.cpp 	\
		../legend/qgslegenditem.cpp 	\
		../legend/qgslegendlayer.cpp 	\
		../legend/qgslegendlayerfile.cpp \
		../legend/qgslegendlayerfilegroup.cpp \
		../legend/qgslegendpropertygroup.cpp \
		../legend/qgslegendpropertyitem.cpp \
		../legend/qgslegendsymbologygroup.cpp \
		../legend/qgslegendsymbologyitem.cpp \
		../legend/qgslegendvectorsymbologyitem.cpp \
                ../composer/qgscomposer.cpp \
		../composer/qgscomposeritem.cpp	       \
		../composer/qgscomposerlabel.cpp	       \
		../composer/qgscomposerpicture.cpp	       \
		../composer/qgscomposermap.cpp	       \
		../composer/qgscomposerscalebar.cpp	       \
		../composer/qgscomposervectorlegend.cpp    \
		../composer/qgscomposerview.cpp	       \
		../composer/qgscomposition.cpp \					
                qgsdbsourceselect.cpp \		 
                qgsnewconnection.cpp \	         
                qgspgquerybuilder.cpp \         
                qgspgutil.cpp # from pg lib


