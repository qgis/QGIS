SET(QGIS_APP_SRCS
  main.cpp
  qgisapp.cpp
  qgisappinterface.cpp
  qgisappstylesheet.cpp
  qgsabout.cpp
  qgssponsors.cpp
  qgsaddattrdialog.cpp
  qgsaddtaborgroup.cpp
  qgsaddjoindialog.cpp
  qgsannotationwidget.cpp
  qgsattributeactiondialog.cpp
  qgsattributetypedialog.cpp
  qgsattributetypeloaddialog.cpp
  qgsattributetabledialog.cpp
  qgsbookmarks.cpp
  qgsbrowserdockwidget.cpp
  qgsclipboard.cpp
  qgsconfigureshortcutsdialog.cpp
  qgscustomization.cpp
  qgscustomprojectiondialog.cpp
  qgsdecorationitem.cpp
  qgsdecorationcopyright.cpp
  qgsdecorationcopyrightdialog.cpp
  qgsdecorationnortharrow.cpp
  qgsdecorationnortharrowdialog.cpp
  qgsdecorationscalebar.cpp
  qgsdecorationscalebardialog.cpp
  qgsdecorationgrid.cpp
  qgsdecorationgriddialog.cpp
  qgsformannotationdialog.cpp
  qgshtmlannotationdialog.cpp
  qgsdelattrdialog.cpp
  qgsdiagramproperties.cpp
  qgsdisplayangle.cpp
  qgsfieldcalculator.cpp
  qgsfieldsproperties.cpp
  qgsidentifyresultsdialog.cpp
  qgsfeatureaction.cpp
  qgslabeldialog.cpp
  qgslabelpropertydialog.cpp
  qgslabelengineconfigdialog.cpp
  qgslabelinggui.cpp
  qgslabelpreview.cpp
  qgsloadstylefromdbdialog.cpp
  qgssavestyletodbdialog.cpp

  qgsmaptooladdfeature.cpp
  qgsmaptooladdpart.cpp
  qgsmaptooladdring.cpp
  qgsmaptoolannotation.cpp
  qgsmaptoolcapture.cpp
  qgsmaptoolchangelabelproperties.cpp
  qgsmaptooldeletering.cpp
  qgsmaptooldeletepart.cpp
  qgsmaptooledit.cpp
  qgsmaptoolfeatureaction.cpp
  qgsmaptoolformannotation.cpp
  qgsmaptoolhtmlannotation.cpp
  qgsmaptoolpinlabels.cpp
  qgsmaptoolshowhidelabels.cpp
  qgsmaptoolidentifyaction.cpp
  qgsmaptoollabel.cpp
  qgsmaptoolmeasureangle.cpp
  qgsmaptoolmovefeature.cpp
  qgsmaptoolmovelabel.cpp
  qgsmaptooloffsetcurve.cpp
  qgsmaptoolreshape.cpp
  qgsmaptoolrotatefeature.cpp
  qgsmaptoolrotatelabel.cpp
  qgsmaptoolrotatepointsymbols.cpp
  qgsmaptoolselect.cpp
  qgsmaptoolselectrectangle.cpp
  qgsmaptoolselectfreehand.cpp
  qgsmaptoolselectpolygon.cpp
  qgsmaptoolselectradius.cpp
  qgsmaptoolselectutils.cpp
  qgsmaptoolsimplify.cpp
  qgsmaptoolsplitfeatures.cpp
  qgsmaptoolsvgannotation.cpp
  qgsmaptooltextannotation.cpp
  qgsmaptoolvertexedit.cpp

  nodetool/qgsmaptoolnodetool.cpp
  nodetool/qgsselectedfeature.cpp
  nodetool/qgsvertexentry.cpp

  qgsmeasuredialog.cpp
  qgsmeasuretool.cpp
  qgsmergeattributesdialog.cpp
  qgsoptions.cpp
  #qgspastetransformations.cpp
  qgspointrotationitem.cpp
  qgspluginmetadata.cpp
  qgspluginregistry.cpp
  qgsprojectlayergroupdialog.cpp
  qgsprojectproperties.cpp
  qgsrastercalcdialog.cpp
  qgsrasterlayerproperties.cpp
  qgstextannotationdialog.cpp
  qgsshortcutsmanager.cpp
  qgssnappingdialog.cpp
  qgssvgannotationdialog.cpp
  qgsundowidget.cpp
  qgstipgui.cpp
  qgstipfactory.cpp
  qgsvectorlayerproperties.cpp
  qgshandlebadlayers.cpp

  composer/qgsattributeselectiondialog.cpp
  composer/qgscomposer.cpp
  composer/qgscomposerarrowwidget.cpp
  composer/qgscomposerhtmlwidget.cpp
  composer/qgscomposeritemwidget.cpp
  composer/qgscomposerlabelwidget.cpp
  composer/qgscomposerpicturewidget.cpp
  composer/qgscomposermanager.cpp
  composer/qgscomposermapwidget.cpp
  composer/qgscomposerscalebarwidget.cpp
  composer/qgscomposershapewidget.cpp
  composer/qgscomposertablewidget.cpp
  composer/qgscomposerlegenditemdialog.cpp
  composer/qgscomposerlegendlayersdialog.cpp
  composer/qgscomposerlegendwidget.cpp
  composer/qgscompositionwidget.cpp
  composer/qgsatlascompositionwidget.cpp

  legend/qgslegendgroup.cpp
  legend/qgslegend.cpp
  legend/qgsapplegendinterface.cpp
  legend/qgslegenditem.cpp
  legend/qgslegendlayer.cpp
  legend/qgslegendpropertygroup.cpp
  legend/qgslegendpropertyitem.cpp
  legend/qgslegendsymbologygroup.cpp
  legend/qgslegendsymbologyitem.cpp
  legend/qgslayerorder.cpp

  ogr/qgsogrhelperfunctions.cpp
  ogr/qgsopenvectorlayerdialog.cpp
  ogr/qgsnewogrconnection.cpp
  ogr/qgsvectorlayersaveasdialog.cpp

  gps/qgsgpsinformationwidget.cpp
  gps/qgsgpsmarker.cpp

  openstreetmap/qgsosmdownloaddialog.cpp
  openstreetmap/qgsosmimportdialog.cpp
  openstreetmap/qgsosmexportdialog.cpp

  pluginmanager/qgspluginmanager.cpp
  pluginmanager/qgspluginmanager_texts.cpp
  pluginmanager/qgsapppluginmanagerinterface.cpp
  pluginmanager/qgspluginsortfilterproxymodel.cpp

  qgsnewspatialitelayerdialog.cpp
)

SET (QGIS_APP_MOC_HDRS
  qgisapp.h
  qgisappinterface.h
  qgisappstylesheet.h
  qgsabout.h
  qgsaddattrdialog.h
  qgsaddjoindialog.h
  qgsaddtaborgroup.h
  qgsannotationwidget.h
  qgsattributeactiondialog.h
  qgsattributetypedialog.h
  qgsattributetypeloaddialog.h
  qgsattributetabledialog.h
  qgsbookmarks.h
  qgsbrowserdockwidget.h
  qgsconfigureshortcutsdialog.h
  qgscustomization.h
  qgscustomprojectiondialog.h
  qgsdecorationitem.h
  qgsdecorationcopyright.h
  qgsdecorationcopyrightdialog.h
  qgsdecorationnortharrow.h
  qgsdecorationnortharrowdialog.h
  qgsdecorationscalebar.h
  qgsdecorationscalebardialog.h
  qgsdecorationgrid.h
  qgsdecorationgriddialog.h
  qgsdelattrdialog.h
  qgsdiagramproperties.h
  qgsdisplayangle.h
  qgsfeatureaction.h
  qgsfieldcalculator.h
  qgsfieldsproperties.h
  qgsformannotationdialog.h
  qgshtmlannotationdialog.h
  qgsidentifyresultsdialog.h
  qgslabeldialog.h
  qgslabelengineconfigdialog.h
  qgslabelinggui.h
  qgslabelpropertydialog.h
  qgsloadstylefromdbdialog.h
  qgssavestyletodbdialog.h
  qgsshortcutsmanager.h

  qgsmaptooladdfeature.h
  qgsmaptoolcapture.h
  qgsmaptooladdpart.h
  qgsmaptooladdring.h
  qgsmaptoolchangelabelproperties.h
  qgsmaptooldeletepart.h
  qgsmaptooldeletering.h
  qgsmaptoolfeatureaction.h
  qgsmaptoolpinlabels.h
  qgsmaptoolshowhidelabels.h
  qgsmaptoolidentifyaction.h
  qgsmaptoolmeasureangle.h
  qgsmaptoolmovefeature.h
  qgsmaptoolmovelabel.h
  qgsmaptooloffsetcurve.h
  qgsmaptoolreshape.h
  qgsmaptoolrotatefeature.h
  qgsmaptoolrotatelabel.h
  qgsmaptoolrotatepointsymbols.h
  qgsmaptoolselectfreehand.h
  qgsmaptoolselect.h
  qgsmaptoolselectpolygon.h
  qgsmaptoolselectradius.h
  qgsmaptoolselectrectangle.h
  qgsmaptoolsimplify.h
  qgsmaptoolsplitfeatures.h
  qgsmaptoolvertexedit.h

  nodetool/qgsmaptoolnodetool.h
  nodetool/qgsselectedfeature.h

  qgsmeasuredialog.h
  qgsmeasuretool.h
  qgsmergeattributesdialog.h
  qgsoptions.h
  #qgspastetransformations.h
  qgsprojectlayergroupdialog.h
  qgsprojectproperties.h
  qgsrastercalcdialog.h
  qgsrasterlayerproperties.h
  qgssnappingdialog.h
  qgssponsors.h
  qgssvgannotationdialog.h
  qgstextannotationdialog.h
  qgstipgui.h
  qgstipfactory.h
  qgsundowidget.h
  qgsvectorlayerproperties.h
  qgshandlebadlayers.h

  composer/qgsattributeselectiondialog.h
  composer/qgscomposer.h
  composer/qgscomposerarrowwidget.h
  composer/qgscomposerhtmlwidget.h
  composer/qgscomposeritemwidget.h
  composer/qgscomposerlabelwidget.h
  composer/qgscomposerlegendwidget.h
  composer/qgscomposermanager.h
  composer/qgscomposermapwidget.h
  composer/qgscomposerpicturewidget.h
  composer/qgscomposerscalebarwidget.h
  composer/qgscomposertablewidget.h
  composer/qgscomposershapewidget.h
  composer/qgscompositionwidget.h
  composer/qgsatlascompositionwidget.h

  legend/qgslegend.h
  legend/qgsapplegendinterface.h
  legend/qgslegendlayer.h
  legend/qgslayerorder.h

  ogr/qgsopenvectorlayerdialog.h
  ogr/qgsnewogrconnection.h
  ogr/qgsvectorlayersaveasdialog.h

  gps/qgsgpsinformationwidget.h

  openstreetmap/qgsosmdownloaddialog.h
  openstreetmap/qgsosmimportdialog.h
  openstreetmap/qgsosmexportdialog.h

  pluginmanager/qgspluginmanager.h
  pluginmanager/qgsapppluginmanagerinterface.h
  pluginmanager/qgspluginsortfilterproxymodel.h

  qgsnewspatialitelayerdialog.h
  )

IF(WITH_INTERNAL_QWTPOLAR)
  IF(QWT_VERSION_STR VERSION_LESS "6.0.0")
    SET(QGIS_APP_SRCS
      ${QGIS_APP_SRCS}
      gps/qwtpolar-0.1/qwt_polar_canvas.cpp
      gps/qwtpolar-0.1/qwt_polar_curve.cpp
      gps/qwtpolar-0.1/qwt_polar_fitter.cpp
      gps/qwtpolar-0.1/qwt_polar_grid.cpp
      gps/qwtpolar-0.1/qwt_polar_itemdict.cpp
      gps/qwtpolar-0.1/qwt_polar_item.cpp
      gps/qwtpolar-0.1/qwt_polar_layout.cpp
      gps/qwtpolar-0.1/qwt_polar_magnifier.cpp
      gps/qwtpolar-0.1/qwt_polar_marker.cpp
      gps/qwtpolar-0.1/qwt_polar_panner.cpp
      gps/qwtpolar-0.1/qwt_polar_plot.cpp
      gps/qwtpolar-0.1/qwt_polar_point.cpp
      gps/qwtpolar-0.1/qwt_polar_spectrogram.cpp
    )

    SET (QGIS_APP_MOC_HDRS
      ${QGIS_APP_MOC_HDRS}
      gps/qwtpolar-0.1/qwt_polar_canvas.h
      gps/qwtpolar-0.1/qwt_polar_magnifier.h
      gps/qwtpolar-0.1/qwt_polar_panner.h
      gps/qwtpolar-0.1/qwt_polar_plot.h
    )

    SET(QWTPOLAR_INCLUDE_DIR gps/qwtpolar-0.1)
  ELSE(QWT_VERSION_STR VERSION_LESS "6.0.0")
    IF(NOT QWT_VERSION_STR VERSION_LESS "6.1.0")
      MESSAGE(FATAL_ERROR "Internal QwtPolar does not support Qwt 6.1 and above")
    ENDIF(NOT QWT_VERSION_STR VERSION_LESS "6.1.0")

    SET(QGIS_APP_SRCS
      ${QGIS_APP_SRCS}
      gps/qwtpolar-1.0/qwt_polar_canvas.cpp
      gps/qwtpolar-1.0/qwt_polar_curve.cpp
      gps/qwtpolar-1.0/qwt_polar_fitter.cpp
      gps/qwtpolar-1.0/qwt_polar_grid.cpp
      gps/qwtpolar-1.0/qwt_polar_item.cpp
      gps/qwtpolar-1.0/qwt_polar_itemdict.cpp
      gps/qwtpolar-1.0/qwt_polar_layout.cpp
      gps/qwtpolar-1.0/qwt_polar_magnifier.cpp
      gps/qwtpolar-1.0/qwt_polar_marker.cpp
      gps/qwtpolar-1.0/qwt_polar_panner.cpp
      gps/qwtpolar-1.0/qwt_polar_picker.cpp
      gps/qwtpolar-1.0/qwt_polar_plot.cpp
      gps/qwtpolar-1.0/qwt_polar_renderer.cpp
      gps/qwtpolar-1.0/qwt_polar_spectrogram.cpp
    )

    SET (QGIS_APP_MOC_HDRS
      ${QGIS_APP_MOC_HDRS}
      gps/qwtpolar-1.0/qwt_polar_canvas.h
      gps/qwtpolar-1.0/qwt_polar_magnifier.h
      gps/qwtpolar-1.0/qwt_polar_panner.h
      gps/qwtpolar-1.0/qwt_polar_picker.h
      gps/qwtpolar-1.0/qwt_polar_plot.h
      gps/qwtpolar-1.0/qwt_polar_renderer.h
    )

    SET(QWTPOLAR_INCLUDE_DIR gps/qwtpolar-1.0)
  ENDIF(QWT_VERSION_STR VERSION_LESS "6.0.0")

  SET(QWTPOLAR_LIBRARY "")
ENDIF(WITH_INTERNAL_QWTPOLAR)


IF (POSTGRES_FOUND)
  IF(HAVE_PGCONFIG)
    ADD_DEFINITIONS(-DHAVE_PGCONFIG=1)
  ENDIF(HAVE_PGCONFIG)
ENDIF (POSTGRES_FOUND)

SET(IMAGE_RCCS ../../images/images.qrc)

QT4_ADD_RESOURCES(IMAGE_RCC_SRCS ${IMAGE_RCCS})

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

IF(PEDANTIC)
  # disable deprecation warnings
  IF(NOT MSVC)
    SET_SOURCE_FILES_PROPERTIES(qgisappinterface.cpp PROPERTIES COMPILE_FLAGS -Wno-deprecated)
  ENDIF(NOT MSVC)
ENDIF(PEDANTIC)

INCLUDE_DIRECTORIES(
  ${CMAKE_CURRENT_SOURCE_DIR} composer legend pluginmanager
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}/../ui
  ${QWT_INCLUDE_DIR}
  ${QT_QTUITOOLS_INCLUDE_DIR}
  ${QEXTSERIALPORT_INCLUDE_DIR}
  ${PYTHON_INCLUDE_PATH}
  ../analysis/raster ../analysis/openstreetmap
  ../core
  ../core/gps
  ../core/composer ../core/raster ../core/symbology-ng
  ../gui ../gui/symbology-ng ../gui/attributetable ../gui/raster
  ../plugins
  ../python
  gps
  openstreetmap
  )

IF (ANDROID)
  INCLUDE_DIRECTORIES(${ANDROID_NDK_TOOLCHAIN_ROOT}/sysroot/usr/include)
ENDIF (ANDROID)

IF (WITH_INTERNAL_SPATIALITE)
  INCLUDE_DIRECTORIES(../core/spatialite/headers)
  INCLUDE_DIRECTORIES(../core/spatialite/headers/spatialite)
ELSE (WITH_INTERNAL_SPATIALITE)
  INCLUDE_DIRECTORIES(${SPATIALITE_INCLUDE_DIR})
  INCLUDE_DIRECTORIES(${SQLITE3_INCLUDE_DIR})
  ADD_DEFINITIONS(-DEXTERNAL_SPATIALITE)
ENDIF (WITH_INTERNAL_SPATIALITE)

INCLUDE_DIRECTORIES(
  ${PROJ_INCLUDE_DIR}
  ${GEOS_INCLUDE_DIR}
  ${GDAL_INCLUDE_DIR}
  ${QWTPOLAR_INCLUDE_DIR}
)

IF (POSTGRES_FOUND)
  INCLUDE_DIRECTORIES(${POSTGRES_INCLUDE_DIR})
ENDIF (POSTGRES_FOUND)

#############
IF (ANDROID)
  ADD_LIBRARY(${QGIS_APP_NAME} SHARED ${QGIS_APP_SRCS} ${QGIS_APP_MOC_SRCS} ${QGIS_APP_HDRS} ${QGIS_APP_MOC_HDRS} ${IMAGE_RCC_SRCS})
ELSE (ANDROID)
  ADD_EXECUTABLE(${QGIS_APP_NAME} MACOSX_BUNDLE WIN32 ${QGIS_APP_SRCS} ${QGIS_APP_MOC_SRCS} ${IMAGE_RCC_SRCS})
ENDIF (ANDROID)

TARGET_LINK_LIBRARIES(${QGIS_APP_NAME}
  ${QWT_LIBRARY}
  ${QT_QTSQL_LIBRARY}
  ${QT_QTUITOOLS_LIBRARY}
  ${QT_QTWEBKIT_LIBRARY}
  #should only be needed for win
  ${QT_QTMAIN_LIBRARY}
  ${QWTPOLAR_LIBRARY}
  ${GDAL_LIBRARY}
  ${PYTHON_LIBRARY}
  qgis_core
  qgis_gui
  qgis_analysis
)

IF (ANDROID)
  TARGET_LINK_LIBRARIES(${QGIS_APP_NAME} log)
ENDIF (ANDROID)

IF(WIN32)
  ADD_DEFINITIONS(-DQWT_DLL)
  TARGET_LINK_LIBRARIES(${QGIS_APP_NAME} DbgHelp)
ENDIF(WIN32)

IF (APPLE)
  TARGET_LINK_LIBRARIES(${QGIS_APP_NAME} ${APP_SERVICES_LIBRARY} )

  SET_TARGET_PROPERTIES(${QGIS_APP_NAME} PROPERTIES
    INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/${QGIS_LIB_DIR}
    INSTALL_RPATH_USE_LINK_PATH true
    MACOSX_BUNDLE_INFO_PLIST "${CMAKE_SOURCE_DIR}/mac/app.info.plist.in")
ENDIF(APPLE)

IF (POSTGRES_FOUND)
  TARGET_LINK_LIBRARIES (${QGIS_APP_NAME} ${POSTGRES_LIBRARY})
ENDIF (POSTGRES_FOUND)

IF(MSVC)
  INSTALL(FILES qgis.ico DESTINATION ${CMAKE_INSTALL_PREFIX}/icons)
ENDIF(MSVC)

INSTALL(TARGETS ${QGIS_APP_NAME}
  BUNDLE DESTINATION ${QGIS_INSTALL_PREFIX}
  RUNTIME DESTINATION ${QGIS_BIN_DIR}
  LIBRARY DESTINATION ${QGIS_LIB_DIR}
  ARCHIVE DESTINATION ${QGIS_LIB_DIR}
  FRAMEWORK DESTINATION ${QGIS_FW_SUBDIR}
  PUBLIC_HEADER DESTINATION ${QGIS_INCLUDE_DIR})

IF (APPLE)
  INSTALL(CODE "EXECUTE_PROCESS(COMMAND /bin/echo -n \"APPLQGIS\" OUTPUT_FILE \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/../PkgInfo\")")
ENDIF (APPLE)
