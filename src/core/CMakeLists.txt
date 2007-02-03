SET(QGIS_GUI_SRCS
../gui/qgisapp.cpp
../gui/qgisiface.cpp
../gui/qgisinterface.cpp
../gui/qgsabout.cpp
../gui/qgsaddattrdialog.cpp
../gui/qgsattributeaction.cpp
../gui/qgsattributeactiondialog.cpp
../gui/qgsattributedialog.cpp
../gui/qgsattributetable.cpp
../gui/qgsattributetabledisplay.cpp
../gui/qgsbookmarks.cpp
../gui/qgsclipboard.cpp
../gui/qgscolorbutton.cpp
../gui/qgscontinuouscolordialog.cpp
../gui/qgscontinuouscolorrenderer.cpp
../gui/qgscoordinatetransform.cpp
../gui/qgscursors.cpp
../gui/qgscustomprojectiondialog.cpp
../gui/qgsdelattrdialog.cpp
../gui/qgsencodingfiledialog.cpp
../gui/qgsfillstylewidget.cpp
../gui/qgsgeomtypedialog.cpp
../gui/qgsgraduatedsymboldialog.cpp
../gui/qgsgraduatedsymbolrenderer.cpp
../gui/qgshelpviewer.cpp
../gui/qgsidentifyresults.cpp
../gui/qgslabel.cpp
../gui/qgslabeldialog.cpp
../gui/qgslayerprojectionselector.cpp
../gui/qgslinestyledialog.cpp
../gui/qgslinestylewidget.cpp
../gui/qgsludialog.cpp
../gui/qgsmapcanvas.cpp
../gui/qgsmapcanvasitem.cpp
../gui/qgsmapcanvasmap.cpp
../gui/qgsmaplayer.cpp
../gui/qgsmaplayerregistry.cpp
../gui/qgsmaplayerset.cpp
../gui/qgsmapoverviewcanvas.cpp
../gui/qgsmaprender.cpp
../gui/qgsmaptoolcapture.cpp
../gui/qgsmaptool.cpp
../gui/qgsmaptoolidentify.cpp
../gui/qgsmaptoolpan.cpp
../gui/qgsmaptoolselect.cpp
../gui/qgsmaptoolvertexedit.cpp
../gui/qgsmaptoolzoom.cpp
../gui/qgsmarkerdialog.cpp
../gui/qgsmeasure.cpp
../gui/qgsmessageviewer.cpp
../gui/qgsnewhttpconnection.cpp
../gui/qgsoptions.cpp
../gui/qgspastetransformations.cpp
../gui/qgspatterndialog.cpp
../gui/qgspgutil.cpp
../gui/qgspluginmanager.cpp
../gui/qgspointstylewidget.cpp
../gui/qgsproject.cpp
../gui/qgsprojectproperties.cpp
../gui/qgsrasterlayerproperties.cpp
../gui/qgsrubberband.cpp
../gui/qgsrunprocess.cpp
../gui/qgssearchquerybuilder.cpp
../gui/qgsserversourceselect.cpp
../gui/qgssinglesymboldialog.cpp
../gui/qgssinglesymbolrenderer.cpp
../gui/qgsspatialrefsys.cpp
../gui/qgsuniquevaluedialog.cpp
../gui/qgsuniquevaluerenderer.cpp
../gui/qgsvectorfilewriter.cpp
../gui/qgsvectorlayer.cpp
../gui/qgsvectorlayerproperties.cpp
../gui/qgsvertexmarker.cpp
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

../raster/qgsrasterlayer.cpp

)

SET(QGIS_GUI_MOC_HDRS
../gui/qgsnewhttpconnection.h
../gui/qgshelpviewer.h
../gui/qgsattributetable.h
../gui/qgssearchquerybuilder.h
../gui/qgsattributeactiondialog.h
../gui/qgslinestyledialog.h
../gui/qgsabout.h
../gui/qgsmaprender.h
../gui/qgsdelattrdialog.h
../gui/qgsmeasure.h
../gui/qgsgeomtypedialog.h
../gui/qgsgraduatedsymboldialog.h
../gui/qgscustomprojectiondialog.h
../gui/qgspastetransformations.h
../gui/qgscontinuouscolordialog.h
../gui/qgssinglesymboldialog.h
../gui/qgsbookmarks.h
../gui/qgsfillstylewidget.h
../gui/qgsmarkerdialog.h
../gui/qgsidentifyresults.h
../gui/qgsvectorlayerproperties.h
../gui/qgsserversourceselect.h
../gui/qgspluginmanager.h
../gui/qgspatterndialog.h
../gui/qgsaddattrdialog.h
../gui/qgsattributetabledisplay.h
../gui/qgsattributedialog.h
../gui/qgsoptions.h
../gui/qgsludialog.h
../gui/qgslinestylewidget.h
../gui/qgisapp.h
../gui/qgisiface.h
../gui/qgisinterface.h
../gui/qgscoordinatetransform.h
../gui/qgsencodingfiledialog.h
../gui/qgslabeldialog.h
../gui/qgslayerprojectionselector.h
../gui/qgsmapcanvas.h
../gui/qgsmapcanvasitem.h
../gui/qgsmapoverviewcanvas.h
../gui/qgsmaplayer.h
../gui/qgsmaplayerregistry.h
../gui/qgsmaptoolidentify.h
../gui/qgspointstylewidget.h
../widgets/projectionselector/qgsprojectionselector.h
../gui/qgsproject.h
../gui/qgsprojectproperties.h
../gui/qgsrasterlayerproperties.h
../gui/qgsrunprocess.h
../gui/qgsuniquevaluedialog.h
../gui/qgsvectorlayer.h
../legend/qgslegend.h
../composer/qgscomposermap.h
../composer/qgscomposerlabel.h
../composer/qgscomposer.h
../composer/qgscomposerpicture.h
../composer/qgscomposerscalebar.h
../composer/qgscomposervectorlegend.h
../composer/qgscomposerview.h
../composer/qgscomposition.h

../raster/qgsrasterlayer.h
)


#############################################################
# sources

SET(QGIS_CORE_SRCS
qgis.cpp
qgsapplication.cpp
qgsbookmarkitem.cpp
qgsclipper.cpp
qgscolortable.cpp
qgscontexthelp.cpp
qgscustomsymbol.cpp
qgsdatamanager.cpp
qgsdatasource.cpp
qgsdistancearea.cpp
qgsexception.cpp
qgsfeatureattribute.cpp
qgsfeature.cpp
qgsfield.cpp
qgsgeometry.cpp
qgsgeometryvertexindex.cpp
qgshttptransaction.cpp
qgslabelattributes.cpp
qgsline.cpp
qgslinesymbol.cpp
qgslogger.cpp
qgsmaptopixel.cpp
qgsmarkercatalogue.cpp
qgsmarkersymbol.cpp
qgsnumericsortlistviewitem.cpp
qgspluginitem.cpp
qgspluginregistry.cpp
../gui/qgspluginmetadata.cpp
qgspoint.cpp
qgspolygonsymbol.cpp
qgsprojectproperty.cpp
qgsprovidercountcalcevent.cpp
qgsproviderextentcalcevent.cpp
qgsprovidermetadata.cpp
qgsproviderregistry.cpp
qgsrangerenderitem.cpp
qgsrasterdataprovider.cpp
qgsrect.cpp
qgsrenderer.cpp
qgsrenderitem.cpp
qgsscalecalculator.cpp
qgssearchstring.cpp
qgssearchtreenode.cpp
qgssymbol.cpp
qgssymbologyutils.cpp
qgsvectordataprovider.cpp


)

ADD_FLEX_FILES(QGIS_CORE_SRCS qgssearchstringlexer.ll)

ADD_BISON_FILES(QGIS_CORE_SRCS qgssearchstringparser.yy)

SET(QGIS_CORE_MOC_HDRS
qgscontexthelp.h
qgsdataprovider.h
qgshttptransaction.h
qgsrasterdataprovider.h

)

IF (POSTGRES_FOUND)
  SET (QGIS_GUI_SRCS ${QGIS_GUI_SRCS}
          ../gui/qgsdbsourceselect.cpp
		  ../gui/qgsnewconnection.cpp
		  ../gui/qgspgquerybuilder.cpp
  )
  SET (QGIS_GUI_MOC_HDRS ${QGIS_GUI_MOC_HDRS}
          ../gui/qgsdbsourceselect.h
		  ../gui/qgsnewconnection.h
		  ../gui/qgspgquerybuilder.h
  )
ENDIF (POSTGRES_FOUND)

QT4_WRAP_CPP(QGIS_GUI_MOC_SRCS ${QGIS_GUI_MOC_HDRS})
QT4_WRAP_CPP(QGIS_CORE_MOC_SRCS ${QGIS_CORE_MOC_HDRS})

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}
  ../raster
  ../legend
  ../gui
  ../composer
  ../plugins
  ../widgets/projectionselector
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}/../ui
  ${QT_QTSVG_INCLUDE_DIR}
  ${PROJ_INCLUDE_DIR}
  ${SQLITE3_INCLUDE_DIR}
  ${GEOS_INCLUDE_DIR}
  ${GDAL_INCLUDE_DIR}
)

IF (WIN32)
  IF (MSVC)
    ADD_DEFINITIONS("-DCORE_EXPORT=__declspec(dllexport)")
  ELSE (MSVC)
    ADD_DEFINITIONS("-UCORE_EXPORT \"-DCORE_EXPORT=__declspec(dllexport)\"")
  ENDIF (MSVC)
ENDIF (WIN32)

#############################################################
# qgis_core library

ADD_LIBRARY(qgis_core SHARED ${QGIS_CORE_SRCS} ${QGIS_CORE_MOC_SRCS} ${QGIS_GUI_SRCS} ${QGIS_GUI_MOC_SRCS} ${INDEX_SRC})

# make sure to create qgssvnversion.h before compiling
ADD_DEPENDENCIES(qgis_core svnversion ui)

# because of htonl
IF (WIN32)
  SET(PLATFORM_LIBRARIES wsock32)
ENDIF (WIN32)

TARGET_LINK_LIBRARIES(qgis_core
  ${QT_LIBRARIES} ${QT_QTSVG_LIBRARY}
  ${PROJ_LIBRARY}
  ${SQLITE3_LIBRARY}
  ${GEOS_LIBRARY}
  ${GDAL_LIBRARY}
  ${PLATFORM_LIBRARIES}
)

IF (POSTGRES_FOUND)
  TARGET_LINK_LIBRARIES (qgis_core ${POSTGRES_LIBRARY})
ENDIF (POSTGRES_FOUND)

INSTALL(TARGETS qgis_core
  RUNTIME DESTINATION ${QGIS_BIN_DIR}
  LIBRARY DESTINATION lib)
