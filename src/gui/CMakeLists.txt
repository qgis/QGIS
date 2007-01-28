
SET(QGIS_GUI_SRCS
qgisapp.cpp
qgisiface.cpp
qgisinterface.cpp
qgsabout.cpp
qgsaddattrdialog.cpp
qgsattributeaction.cpp
qgsattributeactiondialog.cpp
qgsattributedialog.cpp
qgsattributetable.cpp
qgsattributetabledisplay.cpp
qgsbookmarks.cpp
qgsclipboard.cpp
qgscolorbutton.cpp
qgscontinuouscolordialog.cpp
qgscontinuouscolorrenderer.cpp
qgscoordinatetransform.cpp
qgscursors.cpp
qgscustomprojectiondialog.cpp
qgsdelattrdialog.cpp
qgsencodingfiledialog.cpp
qgsfillstylewidget.cpp
qgsgeomtypedialog.cpp
qgsgraduatedsymboldialog.cpp
qgsgraduatedsymbolrenderer.cpp
qgshelpviewer.cpp
qgsidentifyresults.cpp
qgslabel.cpp
qgslabeldialog.cpp
qgslayerprojectionselector.cpp
qgslinestyledialog.cpp
qgslinestylewidget.cpp
qgsludialog.cpp
qgsmapcanvas.cpp
qgsmapcanvasitem.cpp
qgsmapcanvasmap.cpp
qgsmaplayer.cpp
qgsmaplayerregistry.cpp
qgsmaplayerset.cpp
qgsmapoverviewcanvas.cpp
qgsmaprender.cpp
qgsmaptoolcapture.cpp
qgsmaptool.cpp
qgsmaptoolidentify.cpp
qgsmaptoolpan.cpp
qgsmaptoolselect.cpp
qgsmaptoolvertexedit.cpp
qgsmaptoolzoom.cpp
qgsmarkerdialog.cpp
qgsmeasure.cpp
qgsmessageviewer.cpp
qgsnewhttpconnection.cpp
qgsoptions.cpp
qgspastetransformations.cpp
qgspatterndialog.cpp
qgspgutil.cpp
qgspluginmanager.cpp
qgspluginmetadata.cpp
qgspointstylewidget.cpp
qgsproject.cpp
qgsprojectproperties.cpp
qgsrasterlayerproperties.cpp
qgsrubberband.cpp
qgsrunprocess.cpp
qgssearchquerybuilder.cpp
qgsserversourceselect.cpp
qgssinglesymboldialog.cpp
qgssinglesymbolrenderer.cpp
qgsspatialrefsys.cpp
qgsuniquevaluedialog.cpp
qgsuniquevaluerenderer.cpp
qgsvectorfilewriter.cpp
qgsvectorlayer.cpp
qgsvectorlayerproperties.cpp
qgsvertexmarker.cpp
../widgets/projectionselector/qgsprojectionselector.cpp

../composer/qgscomposer.cpp
../composer/qgscomposeritem.cpp
../composer/qgscomposerlabel.cpp
../composer/qgscomposermap.cpp
../composer/qgscomposerpicture.cpp
../composer/qgscomposerscalebar.cpp
../composer/qgscomposervectorlegend.cpp
../composer/qgscomposerview.cpp
../composer/qgscomposition.cpp

../legend/qgslegend.cpp
../legend/qgslegendgroup.cpp
../legend/qgslegenditem.cpp
../legend/qgslegendlayer.cpp
../legend/qgslegendlayerfile.cpp
../legend/qgslegendlayerfilegroup.cpp
../legend/qgslegendpropertygroup.cpp
../legend/qgslegendpropertyitem.cpp
../legend/qgslegendsymbologygroup.cpp
../legend/qgslegendsymbologyitem.cpp
../legend/qgslegendvectorsymbologyitem.cpp

)

SET(QGIS_GUI_MOC_HDRS
qgsnewhttpconnection.h
qgshelpviewer.h
qgsattributetable.h
qgssearchquerybuilder.h
qgsattributeactiondialog.h
qgslinestyledialog.h
qgsabout.h
qgsmaprender.h
qgsdelattrdialog.h
qgsmeasure.h
qgsgeomtypedialog.h
qgsgraduatedsymboldialog.h
qgscustomprojectiondialog.h
qgspastetransformations.h
qgscontinuouscolordialog.h
qgssinglesymboldialog.h
qgsbookmarks.h
qgsfillstylewidget.h
qgsmarkerdialog.h
qgsidentifyresults.h
qgsvectorlayerproperties.h
qgsserversourceselect.h
qgspluginmanager.h
qgspatterndialog.h
qgsaddattrdialog.h
qgsattributetabledisplay.h
qgsattributedialog.h
qgsoptions.h
qgsludialog.h
qgslinestylewidget.h
qgisapp.h
qgisiface.h
qgisinterface.h
qgscoordinatetransform.h
qgsencodingfiledialog.h
qgslabeldialog.h
qgslayerprojectionselector.h
qgsmapcanvas.h
qgsmapcanvasitem.h
qgsmapoverviewcanvas.h
qgsmaplayer.h
qgsmaplayerregistry.h
qgsmaptoolidentify.h
qgspointstylewidget.h
../widgets/projectionselector/qgsprojectionselector.h
qgsproject.h
qgsprojectproperties.h
qgsrasterlayerproperties.h
qgsrunprocess.h
qgsuniquevaluedialog.h
qgsvectorlayer.h
../legend/qgslegend.h
../composer/qgscomposermap.h
../composer/qgscomposerlabel.h
../composer/qgscomposer.h
../composer/qgscomposerpicture.h
../composer/qgscomposerscalebar.h
../composer/qgscomposervectorlegend.h
../composer/qgscomposerview.h
../composer/qgscomposition.h
)

IF (POSTGRES_FOUND)
  SET (QGIS_GUI_SRCS ${QGIS_GUI_SRCS}
          qgsdbsourceselect.cpp
		  qgsnewconnection.cpp
		  qgspgquerybuilder.cpp
  )
  SET (QGIS_GUI_MOC_HDRS ${QGIS_GUI_MOC_HDRS}
          qgsdbsourceselect.h
		  qgsnewconnection.h
		  qgspgquerybuilder.h
  )
ENDIF (POSTGRES_FOUND)


QT4_WRAP_CPP(QGIS_GUI_MOC_SRCS ${QGIS_GUI_MOC_HDRS})

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR} ../core
     ../raster ../legend ../composer ../plugins
     ../widgets/projectionselector
     ${SQLITE3_INCLUDE_DIR}
     ${CMAKE_CURRENT_BINARY_DIR}
     ${CMAKE_CURRENT_BINARY_DIR}/../ui
     ${CMAKE_CURRENT_BINARY_DIR}/../../tools/mapserver_export
)

IF (POSTGRES_FOUND)
  INCLUDE_DIRECTORIES(${POSTGRES_INCLUDE_DIR})
ENDIF (POSTGRES_FOUND)


IF (WIN32)
  IF (MSVC)
    ADD_DEFINITIONS("-DGUI_EXPORT=__declspec(dllexport)")
  ELSE (MSVC)
    ADD_DEFINITIONS("-UGUI_EXPORT \"-DGUI_EXPORT=__declspec(dllexport)\"")
  ENDIF (MSVC)
ENDIF (WIN32)


#############################################################
# qgis_gui library


ADD_LIBRARY(qgis_gui SHARED ${QGIS_GUI_SRCS} ${QGIS_GUI_MOC_SRCS})

# make sure that UI files will be processed first
ADD_DEPENDENCIES(qgis_gui ui)

TARGET_LINK_LIBRARIES(qgis_gui
  ${QT_LIBRARIES}
  ${SQLITE3_LIBRARIES}
  qgis_core
)

INSTALL(TARGETS qgis_gui
  RUNTIME DESTINATION ${QGIS_BIN_DIR}
  LIBRARY DESTINATION lib)

#############

ADD_EXECUTABLE(qgis main.cpp)

TARGET_LINK_LIBRARIES(qgis
  ${QT_LIBRARIES}
  qgis_core
  qgis_gui
)

IF (POSTGRES_FOUND)
  TARGET_LINK_LIBRARIES (qgis ${POSTGRES_LIBRARY})
ENDIF (POSTGRES_FOUND)

IF (APPLE)
  # For Mac OS X, the executable must be at the root of the bundle's executable folder
  INSTALL(TARGETS qgis RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX})
ELSE (APPLE)
  INSTALL(TARGETS qgis RUNTIME DESTINATION ${QGIS_BIN_DIR})
ENDIF (APPLE)
