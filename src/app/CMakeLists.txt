SET(QGIS_APP_SRCS
  main.cpp
  qgisapp.cpp
  qgisappinterface.cpp
  qgsabout.cpp
  qgsaddattrdialog.cpp
  qgsattributeactiondialog.cpp
  qgsattributedialog.cpp
  qgsattributeeditor.cpp
  qgsattributetypedialog.cpp
  qgsattributetypeloaddialog.cpp
  qgsbookmarkitem.cpp
  qgsbookmarks.cpp
  qgsclipboard.cpp
  qgscontinuouscolordialog.cpp
  qgsconfigureshortcutsdialog.cpp
  qgscustomprojectiondialog.cpp
  qgsdbfilterproxymodel.cpp
  qgsdbtablemodel.cpp
  qgsspatialitefilterproxymodel.cpp
  qgsspatialitetablemodel.cpp
  qgsdelattrdialog.cpp
  qgsfieldcalculator.cpp
  qgsnewvectorlayerdialog.cpp
  qgsgraduatedsymboldialog.cpp
  qgshelpviewer.cpp
  qgsidentifyresults.cpp
  qgslabeldialog.cpp
  qgsludialog.cpp
  qgsmaptooladdfeature.cpp
  qgsmaptooladdvertex.cpp
  qgsmaptooladdisland.cpp
  qgsmaptooladdring.cpp
  qgsmaptoolcapture.cpp
  qgsmaptooldeletering.cpp
  qgsmaptooldeletepart.cpp
  qgsmaptooldeletevertex.cpp
  qgsmaptooledit.cpp
  qgsmaptoolidentify.cpp
  qgsmaptoolmovefeature.cpp
  qgsmaptoolmovevertex.cpp
  qgsmaptoolnodetool.cpp
  qgsmaptoolreshape.cpp
  qgsmaptoolrotatepointsymbols.cpp
  qgsmaptoolselect.cpp
  qgsmaptoolsimplify.cpp
  qgsmaptoolsplitfeatures.cpp
  qgsmaptoolvertexedit.cpp
  qgsmeasuredialog.cpp
  qgsmeasuretool.cpp
  qgsmergeattributesdialog.cpp
  qgsnewhttpconnection.cpp
  qgsnumericsortlistviewitem.cpp
  qgsogrsublayersdialog.cpp
  qgsoptions.cpp
  qgspastetransformations.cpp
  qgspointrotationitem.cpp
  qgspluginitem.cpp
  qgspluginmanager.cpp
  qgspluginmetadata.cpp
  qgspluginregistry.cpp
  qgspythondialog.cpp
  qgsprojectproperties.cpp
  qgsrasterlayerproperties.cpp
  qgssearchquerybuilder.cpp
  qgsserversourceselect.cpp
  qgsshortcutsmanager.cpp
  qgssinglesymboldialog.cpp
  qgssnappingdialog.cpp
  qgsundowidget.cpp
  qgsuniquevaluedialog.cpp
  qgsvectorlayerproperties.cpp

  composer/qgscomposer.cpp
  composer/qgscomposeritemwidget.cpp
  composer/qgscomposerlabelwidget.cpp
  composer/qgscomposerpicturewidget.cpp
  composer/qgscomposermapwidget.cpp
  composer/qgscomposerscalebarwidget.cpp
  composer/qgscomposerlegenditemdialog.cpp
  composer/qgscomposerlegendwidget.cpp
  composer/qgscompositionwidget.cpp
  composer/qgsitempositiondialog.cpp

  legend/qgslegendgroup.cpp
  legend/qgslegend.cpp
  legend/qgslegenditem.cpp
  legend/qgslegendlayer.cpp
  legend/qgslegendlayerfile.cpp
  legend/qgslegendlayerfilegroup.cpp
  legend/qgslegendpropertygroup.cpp
  legend/qgslegendpropertyitem.cpp
  legend/qgslegendsymbologygroup.cpp
  legend/qgslegendsymbologyitem.cpp
  legend/qgslegendvectorsymbologyitem.cpp

  ogr/qgsogrhelperfunctions.cpp
  ogr/qgsopenvectorlayerdialog.cpp
  ogr/qgsnewogrconnection.cpp
  
  attributetable/qgsattributetabledialog.cpp
  attributetable/qgsattributetablemodel.cpp
  attributetable/qgsattributetablememorymodel.cpp
  attributetable/qgsattributetableview.cpp
  attributetable/qgsattributetablefiltermodel.cpp
  attributetable/qgsattributetableidcolumnpair.cpp
  attributetable/qgsattributetabledelegate.cpp
  )


SET (QGIS_APP_MOC_HDRS
  qgisapp.h
  qgisappinterface.h
  qgsabout.h
  qgsaddattrdialog.h
  qgsattributeactiondialog.h
  qgsattributedialog.h
  qgsattributeeditor.h
  qgsattributetypedialog.h
  qgsattributetypeloaddialog.h
  qgsbookmarks.h
  qgscontinuouscolordialog.h
  qgsconfigureshortcutsdialog.h
  qgscustomprojectiondialog.h
  qgsdelattrdialog.h
  qgsfieldcalculator.h
  qgsnewvectorlayerdialog.h
  qgsgraduatedsymboldialog.h
  qgshelpviewer.h
  qgsidentifyresults.h
  qgslabeldialog.h
  qgsludialog.h

  qgsmaptoolidentify.h
  qgsmaptoolsplitfeatures.h
  qgsmaptoolvertexedit.h
  qgsmaptooladdfeature.h
  qgsmaptooladdisland.h
  qgsmaptooladdring.h
  qgsmaptoolmovefeature.h
  qgsmaptoolnodetool.h
  qgsmaptoolselect.h
  qgsmaptooladdvertex.h
  qgsmaptooldeletering.h
  qgsmaptooldeletepart.h
  qgsmaptooldeletevertex.h
  qgsmaptoolmovevertex.h
  qgsmaptoolsimplify.h

  qgsmeasuretool.h
  qgsmeasuredialog.h
  qgsmergeattributesdialog.h
  qgsnewhttpconnection.h
  qgsoptions.h
  qgsogrsublayersdialog.h
  qgspastetransformations.h
  qgspluginmanager.h
  qgspythondialog.h
  qgsprojectproperties.h
  qgsrasterlayerproperties.h
  qgssearchquerybuilder.h
  qgsserversourceselect.h
  qgssinglesymboldialog.h
  qgssnappingdialog.h
  qgsuniquevaluedialog.h
  qgsvectorlayerproperties.h
  qgsdbtablemodel.h
  qgsspatialitetablemodel.h
  qgsundowidget.h

  composer/qgscomposer.h
  composer/qgscomposeritemwidget.h
  composer/qgscomposerlabelwidget.h
  composer/qgscomposerlegendwidget.h
  composer/qgscomposermapwidget.h
  composer/qgscomposerpicturewidget.h
  composer/qgscomposerscalebarwidget.h
  composer/qgscompositionwidget.h
  composer/qgsitempositiondialog.h

  legend/qgslegend.h
  legend/qgslegendlayer.h
  legend/qgslegendlayerfile.h
  
  
  ogr/qgsopenvectorlayerdialog.h
  ogr/qgsnewogrconnection.h
  
  attributetable/qgsattributetablemodel.h
  attributetable/qgsattributetablememorymodel.h
  attributetable/qgsattributetabledialog.h
  attributetable/qgsattributetabledelegate.h
  )

IF (POSTGRES_FOUND)
  IF(HAVE_PGCONFIG)
    ADD_DEFINITIONS(-DHAVE_PGCONFIG=1)
  ENDIF(HAVE_PGCONFIG)

  SET (QGIS_APP_SRCS ${QGIS_APP_SRCS}
    qgsdbsourceselect.cpp
    qgsnewconnection.cpp
    qgspgquerybuilder.cpp
    )
  SET (QGIS_APP_MOC_HDRS ${QGIS_APP_MOC_HDRS}
    qgsdbsourceselect.h
    qgsnewconnection.h
    qgspgquerybuilder.h
    )
ENDIF (POSTGRES_FOUND)

IF (HAVE_SPATIALITE)
  SET (QGIS_APP_SRCS ${QGIS_APP_SRCS}
    qgsspatialitesourceselect.cpp
    )
  SET (QGIS_APP_MOC_HDRS ${QGIS_APP_MOC_HDRS}
    qgsspatialitesourceselect.h
    )
ENDIF (HAVE_SPATIALITE)


QT4_WRAP_CPP(QGIS_APP_MOC_SRCS ${QGIS_APP_MOC_HDRS})



IF (WIN32)

  IF (MSVC)
    SET (QGIS_APP_SRCS ${QGIS_APP_SRCS} ${CMAKE_CURRENT_SOURCE_DIR}/qgis_win32.rc)
  ELSE(MSVC)

    # Icon for windows MINGW
    # Note: can't include .rc directly to source files
    # as it's ignored when used MinGW

    IF (NOT WINDRES)
      FIND_PROGRAM(WINDRES windres)
      IF (NOT WINDRES)
        MESSAGE(FATAL_ERROR "windres not found - aborting")
      ENDIF (NOT WINDRES)
    ENDIF (NOT WINDRES)
    #############################################################
    # application icon
    # resource compilation for MinGW
    ADD_CUSTOM_COMMAND( OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/icon.o
      COMMAND ${WINDRES} -I${CMAKE_CURRENT_SOURCE_DIR} -i${CMAKE_CURRENT_SOURCE_DIR}/qgis_win32.rc 
      -o ${CMAKE_CURRENT_BINARY_DIR}/icon.o )
    SET(QGIS_APP_SRCS ${QGIS_APP_SRCS} ${CMAKE_CURRENT_BINARY_DIR}/icon.o)
  ENDIF (MSVC)
ENDIF (WIN32)

INCLUDE_DIRECTORIES(
  ${CMAKE_CURRENT_SOURCE_DIR} composer legend attributetable
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}/../ui
  ../core
  ../core/composer ../core/raster ../core/renderer ../core/symbology
  ../gui
  ../plugins
  ../python
  ${PROJ_INCLUDE_DIR}
  ${GEOS_INCLUDE_DIR}
  ${GDAL_INCLUDE_DIR}
  )

IF (WITH_INTERNAL_SPATIALITE)
  INCLUDE_DIRECTORIES(../core/spatialite/headers/spatialite)
ELSE (WITH_INTERNAL_SPATIALITE)
  INCLUDE_DIRECTORIES(${SQLITE3_INCLUDE_DIR})
ENDIF (WITH_INTERNAL_SPATIALITE)

IF (POSTGRES_FOUND)
  INCLUDE_DIRECTORIES(${POSTGRES_INCLUDE_DIR})
ENDIF (POSTGRES_FOUND)

IF (HAVE_SPATIALITE)
  INCLUDE_DIRECTORIES(${SPATIALITE_INCLUDE_DIR})
ENDIF (HAVE_SPATIALITE)

#############

IF (WIN32)
  # 'WIN32' removes console, which is Bad when debugging
  IF (CMAKE_BUILD_TYPE MATCHES Debug)
    ADD_EXECUTABLE(qgis ${QGIS_APP_SRCS} ${QGIS_APP_MOC_SRCS} ${IMAGE_RCC_SRCS})
  ELSE (CMAKE_BUILD_TYPE MATCHES Debug)
    ADD_EXECUTABLE(qgis WIN32 ${QGIS_APP_SRCS} ${QGIS_APP_MOC_SRCS} ${IMAGE_RCC_SRCS})
  ENDIF (CMAKE_BUILD_TYPE MATCHES Debug)
ELSE (WIN32)
  ADD_EXECUTABLE(qgis ${QGIS_APP_SRCS} ${QGIS_APP_MOC_SRCS} ${IMAGE_RCC_SRCS})
ENDIF (WIN32)

TARGET_LINK_LIBRARIES(qgis
  ${QT_QTCORE_LIBRARY} 
  ${QT_QTGUI_LIBRARY} 
  ${QT_QTXML_LIBRARY} 
  ${QT_QTSVG_LIBRARY} 
  ${QT_QTNETWORK_LIBRARY} 
  ${QT_QTSQL_LIBRARY}
  #should only be needed for win
  ${QT_QTMAIN_LIBRARY} 
  qgis_core
  qgis_gui
  )

IF(NOT WITH_INTERNAL_SPATIALITE)
  TARGET_LINK_LIBRARIES(qgis ${SQLITE_LIBRARY})
ENDIF(NOT WITH_INTERNAL_SPATIALITE)

IF (${QTVERSION} STRLESS "4.3.0")
  TARGET_LINK_LIBRARIES(qgis ${QT_QT3SUPPORT_LIBRARY} )
ENDIF (${QTVERSION} STRLESS "4.3.0")

IF (APPLE)
  TARGET_LINK_LIBRARIES(qgis ${APP_SERVICES_LIBRARY} )
ENDIF (APPLE)

SET_TARGET_PROPERTIES(qgis PROPERTIES
  INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/${QGIS_LIB_DIR}
  INSTALL_RPATH_USE_LINK_PATH true
  )

IF (POSTGRES_FOUND)
  TARGET_LINK_LIBRARIES (qgis ${POSTGRES_LIBRARY})
ENDIF (POSTGRES_FOUND)

IF (SPATIALITE_FOUND)
  TARGET_LINK_LIBRARIES (qgis ${SPATIALITE_LIBRARY})
ENDIF (SPATIALITE_FOUND)

IF (APPLE)
  # For Mac OS X, the executable must be at the root of the bundle's executable folder
  INSTALL(TARGETS qgis RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX})
ELSE (APPLE)
  INSTALL(TARGETS qgis RUNTIME DESTINATION ${QGIS_BIN_DIR})
ENDIF (APPLE)

IF(MSVC)
  INSTALL(FILES qgis.ico DESTINATION ${CMAKE_INSTALL_PREFIX}/icons)
ENDIF(MSVC)

########## Install headers for developers to use against qgis libs
#commented out since app classes dont go into any lib there is little point
#INSTALL(FILES ${CMAKE_BINARY_DIR}/src/ui/ui_qgsmeasurebase.h DESTINATION ${QGIS_INCLUDE_DIR})
#INSTALL(FILES ${QGIS_APP_MOC_HDRS} DESTINATION ${QGIS_INCLUDE_DIR})
