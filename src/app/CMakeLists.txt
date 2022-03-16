add_subdirectory(../../external/libdxfrw dwg/libdxfrw)
if(NOT WIN32)
SUBDIRS(${CMAKE_SOURCE_DIR}/external/qt-unix-signals)
endif()

set(QGIS_APP_SRCS
  ${CMAKE_SOURCE_DIR}/external/nmea/gmath.c
  ${CMAKE_SOURCE_DIR}/external/nmea/time.c

  qgisapp.cpp
  qgisappinterface.cpp
  qgisappstylesheet.cpp
  qgsabout.cpp
  qgsalignrasterdialog.cpp
  qgsappauthrequesthandler.cpp
  qgsappbrowserproviders.cpp
  qgsappcoordinateoperationhandlers.cpp
  qgsapplayertreeviewmenuprovider.cpp
  qgsappwindowmanager.cpp
  qgsappscreenshots.cpp
  qgsanimationexportdialog.cpp
  qgsannotationwidget.cpp
  qgsappsslerrorhandler.cpp
  qgsattributetabledialog.cpp
  qgsbookmarks.cpp
  qgsbookmarkeditordialog.cpp
  qgsclipboard.cpp
  qgscustomization.cpp
  qgsdatumtransformtablewidget.cpp
  qgsdevtoolspanelwidget.cpp
  qgsdiscoverrelationsdialog.cpp
  qgsdxfexportdialog.cpp
  qgsformannotationdialog.cpp
  qgsguivectorlayertools.cpp
  qgshtmlannotationdialog.cpp
  qgsdelattrdialog.cpp
  qgsdisplayangle.cpp
  qgsfirstrundialog.cpp
  qgsfixattributedialog.cpp
  qgsgeometryvalidationservice.cpp
  qgsgeometryvalidationdock.cpp
  qgsgeometryvalidationmodel.cpp
  qgsidentifyresultsdialog.cpp
  qgsfeatureaction.cpp
  qgslayercapabilitiesmodel.cpp
  qgslayernotesmanager.cpp
  qgslayertreegrouppropertieswidget.cpp
  qgslayertreeviewindicatorprovider.cpp
  qgslayertreeviewembeddedindicator.cpp
  qgslayertreeviewfilterindicator.cpp
  qgslayertreeviewlowaccuracyindicator.cpp
  qgslayertreeviewmemoryindicator.cpp
  qgslayertreeviewnocrsindicator.cpp
  qgslayertreeviewnonremovableindicator.cpp
  qgslayertreeviewnotesindicator.cpp
  qgslayertreeviewbadlayerindicator.cpp
  qgslayertreeviewtemporalindicator.cpp
  qgslayertreeviewofflineindicator.cpp
  qgsmapcanvasdockwidget.cpp
  qgsmapsavedialog.cpp
  qgsprojectlistitemdelegate.cpp
  qgsprovidersublayersdialog.cpp
  qgspuzzlewidget.cpp
  qgsversionmigration.cpp
  qgssnappinglayertreemodel.cpp
  qgssnappingwidget.cpp
  qgsstatusbarcoordinateswidget.cpp
  qgsstatusbarmagnifierwidget.cpp
  qgsstatusbarscalewidget.cpp
  qgstemplateprojectsmodel.cpp
  qgstemporalcontrollerdockwidget.cpp
  qgsversioninfo.cpp
  qgsrecentprojectsitemsmodel.cpp
  qgsvectorlayerdigitizingproperties.cpp
  qgswelcomepage.cpp
  qgsdockablewidgethelper.cpp

  qgsmaptooladdfeature.cpp
  qgsmaptooladdpart.cpp
  qgsmaptooladdring.cpp
  qgsmaptoolfillring.cpp
  qgsmaptoolannotation.cpp
  qgsmaptooldeletering.cpp
  qgsmaptooldeletepart.cpp
  qgsmaptooltrimextendfeature.cpp
  qgsmaptoolfeatureaction.cpp
  qgsmaptoolformannotation.cpp
  qgsmaptoolhtmlannotation.cpp
  qgsmaptoolidentifyaction.cpp
  qgsmaptoolmeasureangle.cpp
  qgsmaptoolmeasurebearing.cpp
  qgsmaptoolmovefeature.cpp
  qgsmaptooloffsetcurve.cpp
  qgsmaptooloffsetpointsymbol.cpp
  qgsmaptoolpointsymbol.cpp
  qgsmaptoolreshape.cpp
  qgsmaptoolreverseline.cpp
  qgsmaptoolrotatefeature.cpp
  qgsmaptoolrotatepointsymbols.cpp
  qgsmaptoolscalefeature.cpp
  qgsmaptoolselect.cpp
  qgsmaptoolselectionhandler.cpp
  qgsmaptoolselectutils.cpp
  qgsmaptoolsimplify.cpp
  qgsmaptoolsplitfeatures.cpp
  qgsmaptoolsplitparts.cpp
  qgsmaptoolsvgannotation.cpp
  qgsmaptooltextannotation.cpp

  annotations/qgsannotationitempropertieswidget.cpp
  annotations/qgsannotationlayerproperties.cpp

  decorations/qgsdecorationitem.cpp
  decorations/qgsdecorationtitle.cpp
  decorations/qgsdecorationtitledialog.cpp
  decorations/qgsdecorationcopyright.cpp
  decorations/qgsdecorationcopyrightdialog.cpp
  decorations/qgsdecorationimage.cpp
  decorations/qgsdecorationimagedialog.cpp
  decorations/qgsdecorationlayoutextent.cpp
  decorations/qgsdecorationlayoutextentdialog.cpp
  decorations/qgsdecorationnortharrow.cpp
  decorations/qgsdecorationnortharrowdialog.cpp
  decorations/qgsdecorationscalebar.cpp
  decorations/qgsdecorationscalebardialog.cpp
  decorations/qgsdecorationgrid.cpp
  decorations/qgsdecorationgriddialog.cpp

  pointcloud/qgspointcloudelevationpropertieswidget.cpp
  pointcloud/qgspointcloudlayerproperties.cpp
  pointcloud/qgspointcloudlayerstylewidget.cpp

  raster/qgsrasterelevationpropertieswidget.cpp

  vectortile/qgsvectortilelayerproperties.cpp

  vertextool/qgslockedfeature.cpp
  vertextool/qgsvertexeditor.cpp
  vertextool/qgsvertextool.cpp

  qgslayerstylingwidget.cpp
  qgsmeasuredialog.cpp
  qgsmeasuretool.cpp
  qgsmergeattributesdialog.cpp
  qgspointmarkeritem.cpp
  qgspointrotationitem.cpp
  qgspluginmetadata.cpp
  qgspluginregistry.cpp
  qgsprojectlayergroupdialog.cpp
  qgsprojectproperties.cpp
  qgsrastercalcdialog.cpp
  qgsrelationmanagerdialog.cpp
  qgsrelationadddlg.cpp
  qgsrelationaddpolymorphicdialog.cpp
  qgsselectbyformdialog.cpp
  qgsstatisticalsummarydockwidget.cpp
  qgstextannotationdialog.cpp
  qgssvgannotationdialog.cpp
  qgsundowidget.cpp
  qgsmapthemes.cpp
  qgshandlebadlayers.cpp

  browser/qgsinbuiltdataitemproviders.cpp

  devtools/qgsappdevtoolutils.cpp
  devtools/networklogger/qgsnetworklogger.cpp
  devtools/networklogger/qgsnetworkloggernode.cpp
  devtools/networklogger/qgsnetworkloggerpanelwidget.cpp
  devtools/networklogger/qgsnetworkloggerwidgetfactory.cpp
  devtools/profiler/qgsprofilerpanelwidget.cpp
  devtools/profiler/qgsprofilerwidgetfactory.cpp

  georeferencer/qgsvalidateddoublespinbox.cpp

  labeling/qgslabelpropertydialog.cpp
  labeling/qgsmaptoolchangelabelproperties.cpp
  labeling/qgsmaptoolpinlabels.cpp
  labeling/qgsmaptoolshowhidelabels.cpp
  labeling/qgsmaptoollabel.cpp
  labeling/qgsmaptoolmovelabel.cpp
  labeling/qgsmaptoolrotatelabel.cpp

  layout/qgslayoutappmenuprovider.cpp
  layout/qgslayoutdesignerdialog.cpp
  layout/qgslayoutimagedrophandler.cpp
  layout/qgslayoutmanagerdialog.cpp
  layout/qgsreportfieldgroupsectionwidget.cpp
  layout/qgsreportlayoutsectionwidget.cpp
  layout/qgsreportorganizerwidget.cpp
  layout/qgsreportsectionmodel.cpp
  layout/qgsreportsectionwidget.cpp

  locator/qgsactionlocatorfilter.cpp
  locator/qgsactivelayerfeatureslocatorfilter.cpp
  locator/qgsalllayersfeatureslocatorfilter.cpp
  locator/qgsbookmarklocatorfilter.cpp
  locator/qgsexpressioncalculatorlocatorfilter.cpp
  locator/qgsgotolocatorfilter.cpp
  locator/qgslayertreelocatorfilter.cpp
  locator/qgslayoutlocatorfilter.cpp
  locator/qgsnominatimlocatorfilter.cpp
  locator/qgssettingslocatorfilter.cpp

  locator/qgslocatoroptionswidget.cpp

  maptools/qgsappmaptools.cpp
  maptools/qgsmaptoolsdigitizingtechniquemanager.cpp

  maptools/qgsmaptoolshapecircleabstract.cpp
  maptools/qgsmaptoolshapecircularstringabstract.cpp
  maptools/qgsmaptoolshapeellipseabstract.cpp
  maptools/qgsmaptoolshaperectangleabstract.cpp
  maptools/qgsmaptoolshaperegularpolygonabstract.cpp

  maptools/qgsmaptoolshapecircle2points.cpp
  maptools/qgsmaptoolshapecircle2tangentspoint.cpp
  maptools/qgsmaptoolshapecircle3points.cpp
  maptools/qgsmaptoolshapecircle3tangents.cpp
  maptools/qgsmaptoolshapecirclecenterpoint.cpp
  maptools/qgsmaptoolshapecircularstringradius.cpp
  maptools/qgsmaptoolshapeellipsecenter2points.cpp
  maptools/qgsmaptoolshapeellipsecenterpoint.cpp
  maptools/qgsmaptoolshapeellipseextent.cpp
  maptools/qgsmaptoolshapeellipsefoci.cpp
  maptools/qgsmaptoolshaperectangle3points.cpp
  maptools/qgsmaptoolshaperectanglecenter.cpp
  maptools/qgsmaptoolshaperectangleextent.cpp
  maptools/qgsmaptoolshaperegularpolygon2points.cpp
  maptools/qgsmaptoolshaperegularpolygoncentercorner.cpp
  maptools/qgsmaptoolshaperegularpolygoncenterpoint.cpp

  options/qgsadvancedoptions.cpp
  options/qgscodeeditoroptions.cpp
  options/qgscustomprojectionoptions.cpp
  options/qgsgpsdeviceoptions.cpp
  options/qgsoptions.cpp
  options/qgsoptionsutils.cpp

  gps/qgsgpsbearingitem.cpp
  gps/qgsgpsinformationwidget.cpp
  gps/qgsgpsmarker.cpp

  dwg/qgsdwgimportdialog.cpp
  dwg/qgsdwgimporter.cpp

  pluginmanager/qgspluginmanager.cpp
  pluginmanager/qgspluginmanager_texts.cpp
  pluginmanager/qgsapppluginmanagerinterface.cpp
  pluginmanager/qgspluginsortfilterproxymodel.cpp
  pluginmanager/qgspluginitemdelegate.cpp

  qgssettingstree.cpp
  qgssettingsregistryapp.cpp
  qgsvariantdelegate.cpp
  qgscrashhandler.cpp

  mesh/qgsmeshcalculatordialog.cpp
  mesh/qgsnewmeshlayerdialog.cpp
  mesh/qgsmaptooleditmeshframe.cpp
  mesh/qgsmeshtransformcoordinatesdockwidget.cpp
  mesh/qgsmeshselectbyexpressiondialog.cpp

  vector/qgsvectorelevationpropertieswidget.cpp
)

if (WITH_SPATIALITE)
  set(QGIS_APP_SRCS
    ${QGIS_APP_SRCS}
    qgsnewspatialitelayerdialog.cpp
  )
endif()

if (HAVE_GEOREFERENCER)
  set(QGIS_APP_SRCS
    ${QGIS_APP_SRCS}
    georeferencer/qgsgcpcanvasitem.cpp
    georeferencer/qgsgeorefconfigdialog.cpp
    georeferencer/qgsgeorefdatapoint.cpp
    georeferencer/qgsgeorefdelegates.cpp
    georeferencer/qgsgeorefmainwindow.cpp
    georeferencer/qgsgeoreftooladdpoint.cpp
    georeferencer/qgsgeoreftooldeletepoint.cpp
    georeferencer/qgsgeoreftoolmovepoint.cpp
    georeferencer/qgsgeorefvalidators.cpp
    georeferencer/qgsmapcoordsdialog.cpp
    georeferencer/qgsresidualplotitem.cpp
    georeferencer/qgstransformsettingsdialog.cpp
    georeferencer/qgsgcplist.cpp
    georeferencer/qgsgcplistmodel.cpp
    georeferencer/qgsimagewarper.cpp
    georeferencer/qgsgeoreftransform.cpp
    georeferencer/qgsgcplistwidget.cpp
    georeferencer/qgsrasterchangecoords.cpp
  )
endif()

if (WITH_3D)
  set(QGIS_APP_SRCS
    ${QGIS_APP_SRCS}
    3d/qgs3danimationwidget.cpp
    3d/qgs3danimationexportdialog.cpp
    3d/qgs3dapputils.cpp
    3d/qgs3dicongenerator.cpp
    3d/qgs3dmapcanvas.cpp
    3d/qgs3dmapconfigwidget.cpp
    3d/qgs3dmaptool.cpp
    3d/qgs3dmaptoolidentify.cpp
    3d/qgs3dmaptoolmeasureline.cpp
    3d/qgs3dmeasuredialog.cpp
    3d/qgs3dmodelsourcelineedit.cpp
    3d/qgs3dnavigationwidget.cpp
    3d/qgs3doptions.cpp
    3d/qgsgoochmaterialwidget.cpp
    3d/qgslightswidget.cpp
    3d/qgsline3dsymbolwidget.cpp
    3d/qgsmaterialwidget.cpp
    3d/qgsmesh3dsymbolwidget.cpp
    3d/qgsnullmaterialwidget.cpp
    3d/qgspoint3dsymbolwidget.cpp
    3d/qgspolygon3dsymbolwidget.cpp
    3d/qgsphongmaterialwidget.cpp
    3d/qgsphongtexturedmaterialwidget.cpp
    3d/qgsrulebased3drendererwidget.cpp
    3d/qgssimplelinematerialwidget.cpp
    3d/qgssymbol3dwidget.cpp
    3d/qgsvectorlayer3dpropertieswidget.cpp
    3d/qgsvectorlayer3drendererwidget.cpp
    3d/qgsmeshlayer3drendererwidget.cpp
    3d/qgsmap3dexportwidget.cpp
    layout/qgslayout3dmapwidget.cpp
    3d/qgsskyboxrenderingsettingswidget.cpp
    3d/qgsshadowrenderingsettingswidget.cpp
    3d/qgspointcloud3dsymbolwidget.cpp
    3d/qgspointcloudlayer3drendererwidget.cpp
    3d/qgs3dmapcanvaswidget.cpp
    3d/qgs3dviewsmanagerdialog.cpp
  )
endif()


set (WITH_QWTPOLAR FALSE CACHE BOOL "Determines whether QwtPolar should be built")

if (WITH_QWTPOLAR)
  add_definitions(-DWITH_QWTPOLAR)
  # Try to find QwtPolar on the system
  find_package(QwtPolar)
  # If not found on the system, offer the possibility to build QwtPolar
  # internally
  if(NOT FOUND_QwtPolar)
    set(DEFAULT_WITH_INTERNAL_QWTPOLAR TRUE)
  else()
    set(DEFAULT_WITH_INTERNAL_QWTPOLAR FALSE)
  endif()
  set (WITH_INTERNAL_QWTPOLAR DEFAULT_WITH_INTERNAL_QWTPOLAR CACHE BOOL "Use internal build of QwtPolar")

  if(WITH_INTERNAL_QWTPOLAR)
    set(QGIS_APP_SRCS
      ${QGIS_APP_SRCS}
      ${CMAKE_SOURCE_DIR}/external/qwtpolar-1.1.1/qwt_polar_canvas.cpp
      ${CMAKE_SOURCE_DIR}/external/qwtpolar-1.1.1/qwt_polar_curve.cpp
      ${CMAKE_SOURCE_DIR}/external/qwtpolar-1.1.1/qwt_polar_fitter.cpp
      ${CMAKE_SOURCE_DIR}/external/qwtpolar-1.1.1/qwt_polar_grid.cpp
      ${CMAKE_SOURCE_DIR}/external/qwtpolar-1.1.1/qwt_polar_item.cpp
      ${CMAKE_SOURCE_DIR}/external/qwtpolar-1.1.1/qwt_polar_itemdict.cpp
      ${CMAKE_SOURCE_DIR}/external/qwtpolar-1.1.1/qwt_polar_layout.cpp
      ${CMAKE_SOURCE_DIR}/external/qwtpolar-1.1.1/qwt_polar_magnifier.cpp
      ${CMAKE_SOURCE_DIR}/external/qwtpolar-1.1.1/qwt_polar_marker.cpp
      ${CMAKE_SOURCE_DIR}/external/qwtpolar-1.1.1/qwt_polar_panner.cpp
      ${CMAKE_SOURCE_DIR}/external/qwtpolar-1.1.1/qwt_polar_picker.cpp
      ${CMAKE_SOURCE_DIR}/external/qwtpolar-1.1.1/qwt_polar_plot.cpp
      ${CMAKE_SOURCE_DIR}/external/qwtpolar-1.1.1/qwt_polar_renderer.cpp
      ${CMAKE_SOURCE_DIR}/external/qwtpolar-1.1.1/qwt_polar_spectrogram.cpp
    )

    set(QWTPOLAR_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/external/qwtpolar-1.1.1)

    set(QWTPOLAR_LIBRARY "")
  endif()
else()
  set(QWTPOLAR_LIBRARY "")
  set(QWTPOLAR_INCLUDE_DIR "")
endif()

if (POSTGRES_FOUND)
  if(HAVE_PGCONFIG)
    add_definitions(-DHAVE_PGCONFIG=1)
  endif()
endif()

# Test data dir for QgsAppScreenShots
add_definitions(-DTEST_DATA_DIR="${TEST_DATA_DIR}")

# add test resources, e.g. standard test font
set(TEST_RCCS ../../tests/testdata/testdata.qrc)

if (WIN32)
  set (QGIS_APP_SRCS ${QGIS_APP_SRCS} main.cpp ${TEST_RCCS})
  set (QGIS_APPMAIN_SRCS mainwin.cpp)
  if (NOT MSVC)
    enable_language(RC)
    set(CMAKE_RC_COMPILER_INIT ${CMAKE_GENERATOR_RC})
    set(CMAKE_RC_COMPILE_OBJECT "<CMAKE_RC_COMPILER> -O coff -i <SOURCE> -o <OBJECT>")
    set(QGIS_RC_LIBS -mwindows)
  endif()
  set (QGIS_APPMAIN_SRCS ${QGIS_APPMAIN_SRCS} ${CMAKE_CURRENT_SOURCE_DIR}/qgis_win32.rc)
else()
  set (QGIS_APPMAIN_SRCS main.cpp ${TEST_RCCS})
endif()

if(PEDANTIC)
  # disable deprecation warnings
  if(MSVC)
    set_source_files_properties(
        qgisappinterface.cpp
        ${CMAKE_BINARY_DIR}/src/app/moc_qgisappinterface.cxx
        PROPERTIES COMPILE_FLAGS "/wd4996"
    )
  else()
    set_source_files_properties(
        qgisappinterface.cpp
        ${CMAKE_BINARY_DIR}/src/app/moc_qgisappinterface.cxx
        PROPERTIES COMPILE_FLAGS "-Wno-deprecated-declarations"
    )
  endif()
endif()

if(MSVC)
# -wd4091 Avoid 'typedef' ignored on left of '' when no variable is declared warning in dbghelp.h
set_source_files_properties(
  qgisapp.cpp
  main.cpp
  qgscrashhandler.cpp
  PROPERTIES COMPILE_FLAGS -wd4091)
endif()

if(HAVE_OPENCL)
    include_directories(SYSTEM ${OpenCL_INCLUDE_DIRS})
endif()

if (WITH_PDAL)
  include_directories(SYSTEM ${PDAL_INCLUDE_DIR})
  if(MSVC)
    add_definitions(-DWIN32_LEAN_AND_MEAN)
  endif()
endif()

if(ENABLE_MODELTEST)
  include_directories(../../tests/qt_modeltest)
endif()

if (ANDROID)
  include_directories(SYSTEM ${ANDROID_NDK_TOOLCHAIN_ROOT}/sysroot/usr/include)
endif()

if (APPLE)
  include_directories(${CMAKE_SOURCE_DIR}/src/native/mac)
endif()

if (POSTGRES_FOUND)
  include_directories(SYSTEM ${POSTGRES_INCLUDE_DIR})
endif()

#############
if (ANDROID)
  add_library(${QGIS_APP_NAME} ${LIBRARY_TYPE} ${QGIS_APPMAIN_SRCS})

  # require c++17
  target_compile_features(${QGIS_APP_NAME} PRIVATE cxx_std_17)
else()
  add_executable(${QGIS_APP_NAME} MACOSX_BUNDLE WIN32 ${QGIS_APPMAIN_SRCS})

  # require c++17
  target_compile_features(${QGIS_APP_NAME} PRIVATE cxx_std_17)
endif()

# Putting IMAGE_RCC_SRCS into qgis_app lib is causing problems when the lib is
# loaded (by plugin for example) in test unit (qgis_layoutpicturetest).
add_library(qgis_app ${LIBRARY_TYPE} ${QGIS_APP_SRCS})

# require c++17
target_compile_features(qgis_app PRIVATE cxx_std_17)

target_include_directories(qgis_app PUBLIC
  ${CMAKE_SOURCE_DIR}/external/nmea

  ${CMAKE_SOURCE_DIR}/src/app
  ${CMAKE_SOURCE_DIR}/src/app/decorations
  ${CMAKE_SOURCE_DIR}/src/app/devtools/networklogger
  ${CMAKE_SOURCE_DIR}/src/app/labeling
  ${CMAKE_SOURCE_DIR}/src/app/layout
  ${CMAKE_SOURCE_DIR}/src/app/pluginmanager
  ${CMAKE_SOURCE_DIR}/src/app/gps
  ${CMAKE_SOURCE_DIR}/src/app/dwg
  ${CMAKE_SOURCE_DIR}/src/app/maptools
  ${CMAKE_SOURCE_DIR}/src/app/mesh
  ${CMAKE_SOURCE_DIR}/src/app/locator
  ${CMAKE_SOURCE_DIR}/src/app/pointcloud
  ${CMAKE_SOURCE_DIR}/src/app/vectortile
  ${CMAKE_SOURCE_DIR}/src/plugins
  ${CMAKE_SOURCE_DIR}/src/python
  ${CMAKE_SOURCE_DIR}/src/native

  ${CMAKE_BINARY_DIR}/src/app
)

if (WITH_3D)
  target_include_directories(qgis_app PUBLIC
    ${CMAKE_SOURCE_DIR}/src/app/3d
  )
endif()

target_include_directories(qgis_app SYSTEM PUBLIC
  ${QT_QTUITOOLS_INCLUDE_DIR}
  ${QWTPOLAR_INCLUDE_DIR}
  ${CMAKE_SOURCE_DIR}/external/qt-unix-signals
)

add_dependencies(qgis_gui ui)

target_link_libraries(qgis_app
  ${QWT_LIBRARY}
  ${QWTPOLAR_LIBRARY}
  ${QT_VERSION_BASE}::Sql
  ${QT_VERSION_BASE}::UiTools
  ${OPTIONAL_QTWEBKIT}
  #should only be needed for win
  ${QT_QTMAIN_LIBRARY}
  ${QGIS_RC_LIBS}
  qgis_core
  qgis_gui
  qgis_analysis
  qgis_native
  libdxfrw
)

target_compile_definitions(qgis_app PRIVATE "-DQT_NO_FOREACH")

if (WITH_BINDINGS)
  add_definitions(-DWITH_BINDINGS)
  target_link_libraries(qgis_app qgispython)
endif()

if(ENABLE_MODELTEST)
  target_link_libraries(qgis_app ${QT_VERSION_BASE}::Test)
endif()

if (WITH_3D)
  target_link_libraries(qgis_app qgis_3d)
endif()

if (HAVE_GEOREFERENCER)
  include_directories(SYSTEM
    ${GSL_INCLUDE_DIR}
  )
  target_link_libraries(qgis_app
    ${GSL_LIBRARIES}
  )
endif()

GENERATE_EXPORT_HEADER(
   qgis_app
   BASE_NAME APP
   EXPORT_FILE_NAME qgis_app.h
)


if (NOT ANDROID)
  set_target_properties(qgis_app PROPERTIES
    VERSION ${COMPLETE_VERSION}
    SOVERSION ${COMPLETE_VERSION}
    )
endif()

if (APPLE)
  target_link_libraries(qgis_app ${APP_SERVICES_LIBRARY})
endif()

if(MSVC AND CMAKE_SIZEOF_VOID_P EQUAL 8)
  set_target_properties(qgis_app PROPERTIES STATIC_LIBRARY_FLAGS "/machine:x64")
endif()

# clang-tidy
if(CLANG_TIDY_EXE)
  set_target_properties(
    qgis_app PROPERTIES
    CXX_CLANG_TIDY "${DO_CLANG_TIDY}"
  )
endif()

if(NOT WIN32)
  target_link_libraries(${QGIS_APP_NAME} qgis_app)
endif()

if (ANDROID)
  target_link_libraries(${QGIS_APP_NAME} log)
endif()

if(WIN32)
  add_definitions(-DQWT_DLL)
  add_definitions(-DQSCINTILLA_DLL)
  target_link_libraries(${QGIS_APP_NAME} dbghelp)
  target_link_libraries(qgis_app dbghelp)
endif()

target_link_libraries(${QGIS_APP_NAME} qgis_native)

if (NOT WIN32)
target_link_libraries(${QGIS_APP_NAME} QTSignal)
endif()

if (APPLE)
  set_target_properties(${QGIS_APP_NAME} PROPERTIES
    INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/${QGIS_LIB_DIR}
    INSTALL_RPATH_USE_LINK_PATH true
    MACOSX_BUNDLE_INFO_PLIST "${CMAKE_SOURCE_DIR}/mac/app.info.plist.in")
endif()

if (CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
  find_library(EXECINFO_LIBRARY NAMES execinfo)
  target_link_libraries(${QGIS_APP_NAME} ${EXECINFO_LIBRARY})
endif()

if (POSTGRES_FOUND)
  target_link_libraries (qgis_app ${POSTGRES_LIBRARY})
endif()

if (WITH_PDAL)
  target_link_libraries(qgis_app ${PDAL_LIBRARIES})
endif()

if(MSVC)
  install(FILES qgis.ico qgis-mime.ico qgis-qgs.ico qgis-qlr.ico qgis-qml.ico qgis-qpt.ico DESTINATION ${CMAKE_INSTALL_PREFIX}/icons)
endif()

install(TARGETS qgis_app
  RUNTIME DESTINATION ${QGIS_BIN_DIR}
  LIBRARY DESTINATION ${QGIS_LIB_DIR}
  ARCHIVE DESTINATION ${QGIS_LIB_DIR}
  FRAMEWORK DESTINATION ${QGIS_FW_SUBDIR}
  PUBLIC_HEADER DESTINATION ${QGIS_INCLUDE_DIR})

install(TARGETS ${QGIS_APP_NAME}
  BUNDLE DESTINATION ${QGIS_INSTALL_PREFIX}
  RUNTIME DESTINATION ${QGIS_BIN_DIR}
  LIBRARY DESTINATION ${QGIS_LIB_DIR}
  ARCHIVE DESTINATION ${QGIS_LIB_DIR}
  FRAMEWORK DESTINATION ${QGIS_FW_SUBDIR}
  PUBLIC_HEADER DESTINATION ${QGIS_INCLUDE_DIR})

if (APPLE)
  install(CODE "execute_process(COMMAND /bin/echo -n \"APPLQGIS\" OUTPUT_FILE \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/../PkgInfo\")")
endif()
