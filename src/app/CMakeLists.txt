ADD_SUBDIRECTORY(dwg/libdxfrw)

SET(QGIS_APP_SRCS
  qgisapp.cpp
  qgisappinterface.cpp
  qgisappstylesheet.cpp
  qgsabout.cpp
  qgsalignrasterdialog.cpp
  qgsapplayertreeviewmenuprovider.cpp
  qgsaddattrdialog.cpp
  qgsaddtaborgroup.cpp
  qgsjoindialog.cpp
  qgsannotationwidget.cpp
  qgsattributeactiondialog.cpp
  qgsattributeactionpropertiesdialog.cpp
  qgsattributetypedialog.cpp
  qgsattributetabledialog.cpp
  qgsbookmarks.cpp
  qgsclipboard.cpp
  qgscustomization.cpp
  qgscustomprojectiondialog.cpp
  qgscrashreport.cpp
  qgsdecorationitem.cpp
  qgsdecorationcopyright.cpp
  qgsdecorationcopyrightdialog.cpp
  qgsdecorationlayoutextent.cpp
  qgsdecorationlayoutextentdialog.cpp
  qgsdecorationnortharrow.cpp
  qgsdecorationnortharrowdialog.cpp
  qgsdecorationscalebar.cpp
  qgsdecorationscalebardialog.cpp
  qgsdecorationgrid.cpp
  qgsdecorationgriddialog.cpp
  qgsdiscoverrelationsdlg.cpp
  qgsdxfexportdialog.cpp
  qgsformannotationdialog.cpp
  qgsguivectorlayertools.cpp
  qgshtmlannotationdialog.cpp
  qgsdelattrdialog.cpp
  qgsdiagramproperties.cpp
  qgsdisplayangle.cpp
  qgsfieldcalculator.cpp
  qgsfieldsproperties.cpp
  qgsidentifyresultsdialog.cpp
  qgsfeatureaction.cpp
  qgslabelpropertydialog.cpp
  qgslabelengineconfigdialog.cpp
  qgslabelinggui.cpp
  qgslabelingwidget.cpp
  qgsloadstylefromdbdialog.cpp
  qgsmapcanvasdockwidget.cpp
  qgsmaplayerstyleguiutils.cpp
  qgsmapsavedialog.cpp
  qgsrulebasedlabelingwidget.cpp
  qgssavestyletodbdialog.cpp
  qgssnappinglayertreemodel.cpp
  qgssnappingwidget.cpp
  qgsstatusbarcoordinateswidget.cpp
  qgsstatusbarmagnifierwidget.cpp
  qgsstatusbarscalewidget.cpp
  qgsversioninfo.cpp
  qgswelcomepageitemsmodel.cpp
  qgswelcomepage.cpp

  qgsmaptooladdfeature.cpp
  qgsmaptooladdpart.cpp
  qgsmaptooladdring.cpp
  qgsmaptoolfillring.cpp
  qgsmaptoolannotation.cpp
  qgsmaptoolchangelabelproperties.cpp
  qgsmaptooldeletering.cpp
  qgsmaptooldeletepart.cpp
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
  qgsmaptooloffsetpointsymbol.cpp
  qgsmaptoolpointsymbol.cpp
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
  qgsmaptoolsplitparts.cpp
  qgsmaptoolsvgannotation.cpp
  qgsmaptooltextannotation.cpp

  nodetool/qgsselectedfeature.cpp
  nodetool/qgsvertexentry.cpp
  nodetool/qgsnodeeditor.cpp
  nodetool/qgsnodetool.cpp

  qgslayerstylingwidget.cpp
  qgsmeasuredialog.cpp
  qgsmeasuretool.cpp
  qgsmergeattributesdialog.cpp
  qgsoptions.cpp
  qgspointmarkeritem.cpp
  qgspointrotationitem.cpp
  qgspluginmetadata.cpp
  qgspluginregistry.cpp
  qgsprojectlayergroupdialog.cpp
  qgsprojectproperties.cpp
  qgsrastercalcdialog.cpp
  qgsrasterlayerproperties.cpp
  qgsrelationmanagerdialog.cpp
  qgsrelationadddlg.cpp
  qgsselectbyformdialog.cpp
  qgsstatisticalsummarydockwidget.cpp
  qgstextannotationdialog.cpp
  qgssvgannotationdialog.cpp
  qgsundowidget.cpp
  qgsvectorlayerproperties.cpp
  qgsmapthemes.cpp
  qgshandlebadlayers.cpp
  qgsmaptooladdcircularstring.cpp
  qgsmaptoolcircularstringcurvepoint.cpp
  qgsmaptoolcircularstringradius.cpp

  composer/qgsattributeselectiondialog.cpp
  composer/qgscomposer.cpp
  composer/qgscomposerarrowwidget.cpp
  composer/qgscomposerattributetablewidget.cpp
  composer/qgscomposerhtmlwidget.cpp
  composer/qgscomposerimageexportoptionsdialog.cpp
  composer/qgscomposeritemwidget.cpp
  composer/qgscomposerlabelwidget.cpp
  composer/qgscomposerpicturewidget.cpp
  composer/qgscomposermanager.cpp
  composer/qgscomposermapgridwidget.cpp
  composer/qgscomposermapwidget.cpp
  composer/qgscomposerscalebarwidget.cpp
  composer/qgscomposershapewidget.cpp
  composer/qgscomposerpolygonwidget.cpp
  composer/qgscomposerpolylinewidget.cpp
  composer/qgscomposertablebackgroundcolorsdialog.cpp
  composer/qgscomposerlegendlayersdialog.cpp
  composer/qgscomposerlegendwidget.cpp
  composer/qgscompositionwidget.cpp
  composer/qgsatlascompositionwidget.cpp

  layout/qgslayoutaddpagesdialog.cpp
  layout/qgslayoutdesignerdialog.cpp
  layout/qgslayoutguidewidget.cpp
  layout/qgslayoutappmenuprovider.cpp
  layout/qgslayoutpagepropertieswidget.cpp
  layout/qgslayoutpropertieswidget.cpp

  locator/qgsinbuiltlocatorfilters.cpp
  locator/qgslocatoroptionswidget.cpp

  gps/qgsgpsinformationwidget.cpp
  gps/qgsgpsmarker.cpp

  openstreetmap/qgsosmdownloaddialog.cpp
  openstreetmap/qgsosmimportdialog.cpp
  openstreetmap/qgsosmexportdialog.cpp

  dwg/qgsdwgimportdialog.cpp
  dwg/qgsdwgimporter.cpp

  pluginmanager/qgspluginmanager.cpp
  pluginmanager/qgspluginmanager_texts.cpp
  pluginmanager/qgsapppluginmanagerinterface.cpp
  pluginmanager/qgspluginsortfilterproxymodel.cpp
  pluginmanager/qgspluginitemdelegate.cpp

  qgsnewspatialitelayerdialog.cpp

  qgssettingstree.cpp
  qgsvariantdelegate.cpp
  qgscrashdialog.cpp
  qgscrashhandler.cpp
)

SET (QGIS_APP_MOC_HDRS
  qgisapp.h
  qgisappinterface.h
  qgisappstylesheet.h
  qgsabout.h
  qgsaddattrdialog.h
  qgsalignrasterdialog.h
  qgsjoindialog.h
  qgsaddtaborgroup.h
  qgsannotationwidget.h
  qgsapplayertreeviewmenuprovider.h
  qgsattributeactiondialog.h
  qgsattributeactionpropertiesdialog.h
  qgsattributetypedialog.h
  qgsattributetabledialog.h
  qgsbookmarks.h
  qgsclipboard.h
  qgscustomization.h
  qgscustomprojectiondialog.h
  qgsdecorationitem.h
  qgsdecorationcopyright.h
  qgsdecorationcopyrightdialog.h
  qgsdecorationlayoutextent.h
  qgsdecorationlayoutextentdialog.h
  qgsdecorationnortharrow.h
  qgsdecorationnortharrowdialog.h
  qgsdecorationscalebar.h
  qgsdecorationscalebardialog.h
  qgsdecorationgrid.h
  qgsdecorationgriddialog.h
  qgsdelattrdialog.h
  qgsdiagramproperties.h
  qgsdiscoverrelationsdlg.h
  qgsdisplayangle.h
  qgsdxfexportdialog.h
  qgsfeatureaction.h
  qgsfieldcalculator.h
  qgsfieldsproperties.h
  qgsformannotationdialog.h
  qgsguivectorlayertools.h
  qgshtmlannotationdialog.h
  qgsidentifyresultsdialog.h
  qgslabelengineconfigdialog.h
  qgslabelinggui.h
  qgslabelingwidget.h
  qgslabelpropertydialog.h
  qgsloadstylefromdbdialog.h
  qgsmapcanvasdockwidget.h
  qgsmaplayerstyleguiutils.h
  qgsmapsavedialog.h
  qgsrulebasedlabelingwidget.h
  qgssavestyletodbdialog.h
  qgssnappinglayertreemodel.h
  qgssnappingwidget.h
  qgsstatusbarcoordinateswidget.h
  qgsstatusbarmagnifierwidget.h
  qgsstatusbarscalewidget.h
  qgsversioninfo.h
  qgswelcomepageitemsmodel.h
  qgswelcomepage.h

  qgsmaptooladdfeature.h
  qgsmaptoolannotation.h
  qgsmaptoolcircularstringradius.h
  qgsmaptooladdpart.h
  qgsmaptooladdring.h
  qgsmaptoolfillring.h
  qgsmaptoolchangelabelproperties.h
  qgsmaptooldeletepart.h
  qgsmaptooldeletering.h
  qgsmaptoolfeatureaction.h
  qgsmaptoolformannotation.h
  qgsmaptoolhtmlannotation.h
  qgsmaptoolpinlabels.h
  qgsmaptoolshowhidelabels.h
  qgsmaptoolidentifyaction.h
  qgsmaptoolmeasureangle.h
  qgsmaptoolmovefeature.h
  qgsmaptoolmovelabel.h
  qgsmaptoollabel.h
  qgsmaptooloffsetcurve.h
  qgsmaptooloffsetpointsymbol.h
  qgsmaptoolpointsymbol.h
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
  qgsmaptoolsplitparts.h
  qgsmaptoolsvgannotation.h
  qgsmaptooltextannotation.h
  qgsmaptooladdcircularstring.h
  qgsmaptoolcircularstringcurvepoint.h

  nodetool/qgsselectedfeature.h
  nodetool/qgsnodeeditor.h
  nodetool/qgsnodetool.h

  qgslayerstylingwidget.h
  qgsmeasuredialog.h
  qgsmeasuretool.h
  qgsmergeattributesdialog.h
  qgsoptions.h
  qgsprojectlayergroupdialog.h
  qgsprojectproperties.h
  qgsrastercalcdialog.h
  qgsrasterlayerproperties.h
  qgsrelationmanagerdialog.h
  qgsrelationadddlg.h
  qgsselectbyformdialog.h
  qgsstatisticalsummarydockwidget.h
  qgssvgannotationdialog.h
  qgstextannotationdialog.h
  qgsundowidget.h
  qgsvectorlayerproperties.h
  qgsmapthemes.h
  qgshandlebadlayers.h

  composer/qgsattributeselectiondialog.h
  composer/qgscomposer.h
  composer/qgscomposerarrowwidget.h
  composer/qgscomposerattributetablewidget.h
  composer/qgscomposerhtmlwidget.h
  composer/qgscomposerimageexportoptionsdialog.h
  composer/qgscomposeritemwidget.h
  composer/qgscomposerlabelwidget.h
  composer/qgscomposerlegendwidget.h
  composer/qgscomposerlegendlayersdialog.h
  composer/qgscomposermanager.h
  composer/qgscomposermapgridwidget.h
  composer/qgscomposermapwidget.h
  composer/qgscomposerpicturewidget.h
  composer/qgscomposerscalebarwidget.h
  composer/qgscomposertablebackgroundcolorsdialog.h
  composer/qgscomposershapewidget.h
  composer/qgscomposerpolygonwidget.h
  composer/qgscomposerpolylinewidget.h
  composer/qgscompositionwidget.h
  composer/qgsatlascompositionwidget.h

  layout/qgslayoutaddpagesdialog.h
  layout/qgslayoutappmenuprovider.h
  layout/qgslayoutdesignerdialog.h
  layout/qgslayoutguidewidget.h
  layout/qgslayoutpagepropertieswidget.h
  layout/qgslayoutpropertieswidget.h

  locator/qgsinbuiltlocatorfilters.h
  locator/qgslocatoroptionswidget.h

  gps/qgsgpsinformationwidget.h

  openstreetmap/qgsosmdownloaddialog.h
  openstreetmap/qgsosmimportdialog.h
  openstreetmap/qgsosmexportdialog.h

  dwg/qgsdwgimportdialog.h

  pluginmanager/qgspluginmanager.h
  pluginmanager/qgsapppluginmanagerinterface.h
  pluginmanager/qgspluginsortfilterproxymodel.h
  pluginmanager/qgspluginitemdelegate.h

  qgsnewspatialitelayerdialog.h

  qgssettingstree.h
  qgsvariantdelegate.h
  qgscrashdialog.h
  )

SET (WITH_QWTPOLAR FALSE CACHE BOOL "Determines whether QwtPolar should be built")

IF (WITH_QWTPOLAR)
  ADD_DEFINITIONS(-DWITH_QWTPOLAR)
  # Try to find QwtPolar on the system
  FIND_PACKAGE(QwtPolar)
  # If not found on the system, offer the possibility to build QwtPolar
  # internally
  IF(NOT FOUND_QwtPolar)
    SET(DEFAULT_WITH_INTERNAL_QWTPOLAR TRUE)
  ELSE(NOT FOUND_QwtPolar)
    SET(DEFAULT_WITH_INTERNAL_QWTPOLAR FALSE)
  ENDIF(NOT FOUND_QwtPolar)
  SET (WITH_INTERNAL_QWTPOLAR DEFAULT_WITH_INTERNAL_QWTPOLAR CACHE BOOL "Use internal build of QwtPolar")

  IF(WITH_INTERNAL_QWTPOLAR)
    SET(QGIS_APP_SRCS
      ${QGIS_APP_SRCS}
      gps/qwtpolar-1.1.1/qwt_polar_canvas.cpp
      gps/qwtpolar-1.1.1/qwt_polar_curve.cpp
      gps/qwtpolar-1.1.1/qwt_polar_fitter.cpp
      gps/qwtpolar-1.1.1/qwt_polar_grid.cpp
      gps/qwtpolar-1.1.1/qwt_polar_item.cpp
      gps/qwtpolar-1.1.1/qwt_polar_itemdict.cpp
      gps/qwtpolar-1.1.1/qwt_polar_layout.cpp
      gps/qwtpolar-1.1.1/qwt_polar_magnifier.cpp
      gps/qwtpolar-1.1.1/qwt_polar_marker.cpp
      gps/qwtpolar-1.1.1/qwt_polar_panner.cpp
      gps/qwtpolar-1.1.1/qwt_polar_picker.cpp
      gps/qwtpolar-1.1.1/qwt_polar_plot.cpp
      gps/qwtpolar-1.1.1/qwt_polar_renderer.cpp
      gps/qwtpolar-1.1.1/qwt_polar_spectrogram.cpp
    )

    SET (QGIS_APP_MOC_HDRS
      ${QGIS_APP_MOC_HDRS}
      gps/qwtpolar-1.1.1/qwt_polar_canvas.h
      gps/qwtpolar-1.1.1/qwt_polar_magnifier.h
      gps/qwtpolar-1.1.1/qwt_polar_panner.h
      gps/qwtpolar-1.1.1/qwt_polar_picker.h
      gps/qwtpolar-1.1.1/qwt_polar_plot.h
      gps/qwtpolar-1.1.1/qwt_polar_renderer.h
    )

    SET(QWTPOLAR_INCLUDE_DIR gps/qwtpolar-1.1.1)

    SET(QWTPOLAR_LIBRARY "")
  ENDIF(WITH_INTERNAL_QWTPOLAR)
ELSE(WITH_QWTPOLAR)
  SET(QWTPOLAR_LIBRARY "")
  SET(QWTPOLAR_INCLUDE_DIR "")
ENDIF(WITH_QWTPOLAR)

IF (POSTGRES_FOUND)
  IF(HAVE_PGCONFIG)
    ADD_DEFINITIONS(-DHAVE_PGCONFIG=1)
  ENDIF(HAVE_PGCONFIG)
ENDIF (POSTGRES_FOUND)

SET(IMAGE_RCCS ../../images/images.qrc)

QT5_ADD_RESOURCES(IMAGE_RCC_SRCS ${IMAGE_RCCS})

# add test resources, e.g. standard test font
SET(TEST_RCCS ../../tests/testdata/testdata.qrc)
QT5_ADD_RESOURCES(TEST_RCC_SRCS ${TEST_RCCS})

QT5_WRAP_CPP(QGIS_APP_MOC_SRCS ${QGIS_APP_MOC_HDRS})

IF (WIN32)
  FIND_PACKAGE(Qt5WinExtras)

  SET (QGIS_APP_SRCS ${QGIS_APP_SRCS} main.cpp ${IMAGE_RCC_SRCS} ${TEST_RCC_SRCS})
  SET (QGIS_APPMAIN_SRCS mainwin.cpp)

  IF (MSVC)
    SET (QGIS_APPMAIN_SRCS ${QGIS_APPMAIN_SRCS} ${CMAKE_CURRENT_SOURCE_DIR}/qgis_win32.rc)
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
    SET(QGIS_APPMAIN_SRCS ${QGIS_APPMAIN_SRCS} ${CMAKE_CURRENT_BINARY_DIR}/icon.o)
  ENDIF (MSVC)
ELSE(WIN32)
  SET (QGIS_APPMAIN_SRCS main.cpp ${IMAGE_RCC_SRCS} ${TEST_RCC_SRCS})
ENDIF (WIN32)

IF(PEDANTIC)
  # disable deprecation warnings
  IF(MSVC)
    SET_SOURCE_FILES_PROPERTIES(
        qgisappinterface.cpp
        ${CMAKE_BINARY_DIR}/src/app/moc_qgisappinterface.cxx
    PROPERTIES COMPILE_FLAGS "/wd4996"
    )
  ELSE(MSVC)
    SET_SOURCE_FILES_PROPERTIES(
        qgisappinterface.cpp
        ${CMAKE_BINARY_DIR}/src/app/moc_qgisappinterface.cxx
        PROPERTIES COMPILE_FLAGS "-Wno-deprecated-declarations"
    )
  ENDIF(MSVC)
ENDIF(PEDANTIC)

IF(MSVC)
# -wd4091 Avoid 'typedef' ignored on left of '' when no variable is declared warning in DbgHelp.h
SET_SOURCE_FILES_PROPERTIES(
  qgisapp.cpp
  main.cpp
  qgscrashhandler.cpp
  PROPERTIES COMPILE_FLAGS -wd4091)
ENDIF(MSVC)

INCLUDE_DIRECTORIES(
  ${CMAKE_SOURCE_DIR}/src/app
  ${CMAKE_SOURCE_DIR}/src/app/composer
  ${CMAKE_SOURCE_DIR}/src/app/layout
  ${CMAKE_SOURCE_DIR}/src/app/pluginmanager
  ${CMAKE_SOURCE_DIR}/src/app/gps
  ${CMAKE_SOURCE_DIR}/src/app/openstreetmap
  ${CMAKE_SOURCE_DIR}/src/app/dwg
  ${CMAKE_SOURCE_DIR}/src/app/dwg/libdxfrw
  ${CMAKE_SOURCE_DIR}/src/app/locator
  ${CMAKE_SOURCE_DIR}/src/analysis/raster
  ${CMAKE_SOURCE_DIR}/src/analysis/openstreetmap
  ${CMAKE_SOURCE_DIR}/src/core
  ${CMAKE_SOURCE_DIR}/src/core/annotations
  ${CMAKE_SOURCE_DIR}/src/core/auth
  ${CMAKE_SOURCE_DIR}/src/core/expression
  ${CMAKE_SOURCE_DIR}/src/core/gps
  ${CMAKE_SOURCE_DIR}/src/core/composer
  ${CMAKE_SOURCE_DIR}/src/core/dxf
  ${CMAKE_SOURCE_DIR}/src/core/geometry
  ${CMAKE_SOURCE_DIR}/src/core/metadata
  ${CMAKE_SOURCE_DIR}/src/core/layertree
  ${CMAKE_SOURCE_DIR}/src/core/providers/memory
  ${CMAKE_SOURCE_DIR}/src/core/raster
  ${CMAKE_SOURCE_DIR}/src/core/scalebar
  ${CMAKE_SOURCE_DIR}/src/core/symbology
  ${CMAKE_SOURCE_DIR}/src/gui
  ${CMAKE_SOURCE_DIR}/src/gui/symbology
  ${CMAKE_SOURCE_DIR}/src/gui/attributetable
  ${CMAKE_SOURCE_DIR}/src/gui/auth
  ${CMAKE_SOURCE_DIR}/src/gui/geonode
  ${CMAKE_SOURCE_DIR}/src/gui/ogr
  ${CMAKE_SOURCE_DIR}/src/gui/raster
  ${CMAKE_SOURCE_DIR}/src/gui/editorwidgets
  ${CMAKE_SOURCE_DIR}/src/gui/editorwidgets/core
  ${CMAKE_SOURCE_DIR}/src/gui/layertree
  ${CMAKE_SOURCE_DIR}/src/gui/locator
  ${CMAKE_SOURCE_DIR}/src/plugins
  ${CMAKE_SOURCE_DIR}/src/python
  ${CMAKE_SOURCE_DIR}/src/native

  ${CMAKE_BINARY_DIR}/src/core
  ${CMAKE_BINARY_DIR}/src/gui
  ${CMAKE_BINARY_DIR}/src/python
  ${CMAKE_BINARY_DIR}/src/analysis
  ${CMAKE_BINARY_DIR}/src/app
  ${CMAKE_BINARY_DIR}/src/ui
  ${CMAKE_BINARY_DIR}/src/native
)

INCLUDE_DIRECTORIES(SYSTEM
  ${QWT_INCLUDE_DIR}
  ${QT_QTUITOOLS_INCLUDE_DIR}
  ${QSCINTILLA_INCLUDE_DIR}
  ${QEXTSERIALPORT_INCLUDE_DIR}
)
INCLUDE_DIRECTORIES(
  ../analysis/raster
  ../analysis/openstreetmap
  ../core
  ../core/annotations
  ../core/auth
  ../core/gps
  ../core/composer
  ../core/dxf
  ../core/geometry
  ../core/layout
  ../core/metadata
  ../core/layertree
  ../core/providers/memory
  ../core/raster
  ../core/scalebar
  ../core/symbology
  ../gui
  ../gui/symbology
  ../gui/attributetable
  ../gui/auth
  ../gui/geonode
  ../gui/ogr
  ../gui/raster
  ../gui/editorwidgets
  ../gui/editorwidgets/core
  ../gui/layertree
  ../gui/layout
  ../plugins
  ../python
  gps
  openstreetmap
  dwg
  dwg/libdxfrw
  ${CMAKE_SOURCE_DIR}/src/native
  ${CMAKE_BINARY_DIR}/src/native
)
INCLUDE_DIRECTORIES(SYSTEM
  ${SPATIALITE_INCLUDE_DIR}
  ${SQLITE3_INCLUDE_DIR}
  ${PROJ_INCLUDE_DIR}
  ${GEOS_INCLUDE_DIR}
  ${GDAL_INCLUDE_DIR}
  ${QWTPOLAR_INCLUDE_DIR}
  ${QCA_INCLUDE_DIR}
  ${QTKEYCHAIN_INCLUDE_DIR}
)

IF(ENABLE_MODELTEST)
  INCLUDE_DIRECTORIES(../../tests/qt_modeltest)
ENDIF(ENABLE_MODELTEST)

IF (ANDROID)
  INCLUDE_DIRECTORIES(SYSTEM ${ANDROID_NDK_TOOLCHAIN_ROOT}/sysroot/usr/include)
ENDIF (ANDROID)

IF (APPLE)
  INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src/native/mac)
ENDIF (APPLE)

IF (POSTGRES_FOUND)
  INCLUDE_DIRECTORIES(SYSTEM ${POSTGRES_INCLUDE_DIR})
ENDIF (POSTGRES_FOUND)

#############
IF (ANDROID)
  ADD_LIBRARY(${QGIS_APP_NAME} SHARED ${QGIS_APPMAIN_SRCS})
ELSE (ANDROID)
  ADD_EXECUTABLE(${QGIS_APP_NAME} MACOSX_BUNDLE WIN32 ${QGIS_APPMAIN_SRCS})
ENDIF (ANDROID)

# Putting IMAGE_RCC_SRCS into qgis_app lib is causing problems when the lib is
# loaded (by plugin for example) in test unit (qgis_composerpicturetest).
ADD_LIBRARY(qgis_app SHARED ${QGIS_APP_SRCS} ${QGIS_APP_MOC_SRCS} ${QGIS_APP_HDRS} ${QGIS_APP_MOC_HDRS})

TARGET_LINK_LIBRARIES(qgis_app
  ${QWT_LIBRARY}
  ${QT_QTSQL_LIBRARY}
  ${QT_QTUITOOLS_LIBRARY}
  ${OPTIONAL_QTWEBKIT}
  #should only be needed for win
  ${QT_QTMAIN_LIBRARY}
  ${QWTPOLAR_LIBRARY}
  ${GDAL_LIBRARY}
  qgis_core
  qgis_gui
  qgis_analysis
  qgis_native
  libdxfrw
)

GENERATE_EXPORT_HEADER(
   qgis_app
   BASE_NAME APP
   EXPORT_FILE_NAME qgis_app.h
)

IF (NOT ANDROID)
  SET_TARGET_PROPERTIES(qgis_app PROPERTIES
    VERSION ${COMPLETE_VERSION}
    SOVERSION ${COMPLETE_VERSION}
    )
ENDIF (NOT ANDROID)

IF (APPLE)
  TARGET_LINK_LIBRARIES(qgis_app ${APP_SERVICES_LIBRARY})
ENDIF(APPLE)


if(MSVC AND CMAKE_SIZEOF_VOID_P EQUAL 8)
  SET_TARGET_PROPERTIES(qgis_app PROPERTIES STATIC_LIBRARY_FLAGS "/machine:x64")
ENDIF(MSVC AND CMAKE_SIZEOF_VOID_P EQUAL 8)

IF(NOT WIN32)
  TARGET_LINK_LIBRARIES(${QGIS_APP_NAME} qgis_app)
ENDIF(NOT WIN32)

IF (ANDROID)
  TARGET_LINK_LIBRARIES(${QGIS_APP_NAME} log)
ENDIF (ANDROID)

IF(WIN32)
  ADD_DEFINITIONS(-DQWT_DLL)
  ADD_DEFINITIONS(-DQSCINTILLA_DLL)
  TARGET_LINK_LIBRARIES(${QGIS_APP_NAME} DbgHelp)
  TARGET_LINK_LIBRARIES(qgis_app DbgHelp Qt5::WinExtras)
ENDIF(WIN32)

TARGET_LINK_LIBRARIES(${QGIS_APP_NAME} qgis_native)

IF (APPLE)
  SET_TARGET_PROPERTIES(${QGIS_APP_NAME} PROPERTIES
    INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/${QGIS_LIB_DIR}
    INSTALL_RPATH_USE_LINK_PATH true
    MACOSX_BUNDLE_INFO_PLIST "${CMAKE_SOURCE_DIR}/mac/app.info.plist.in")
ENDIF(APPLE)

IF (CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
  FIND_LIBRARY(EXECINFO_LIBRARY NAMES execinfo)
  TARGET_LINK_LIBRARIES(${QGIS_APP_NAME} ${EXECINFO_LIBRARY})
ENDIF (CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")

IF (POSTGRES_FOUND)
  TARGET_LINK_LIBRARIES (qgis_app ${POSTGRES_LIBRARY})
ENDIF (POSTGRES_FOUND)

IF(MSVC)
  INSTALL(FILES qgis.ico qgis-mime.ico qgis-qgs.ico qgis-qlr.ico qgis-qml.ico qgis-qpt.ico DESTINATION ${CMAKE_INSTALL_PREFIX}/icons)
ENDIF(MSVC)

INSTALL(TARGETS qgis_app
  RUNTIME DESTINATION ${QGIS_BIN_DIR}
  LIBRARY DESTINATION ${QGIS_LIB_DIR}
  ARCHIVE DESTINATION ${QGIS_LIB_DIR}
  FRAMEWORK DESTINATION ${QGIS_FW_SUBDIR}
  PUBLIC_HEADER DESTINATION ${QGIS_INCLUDE_DIR})

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
