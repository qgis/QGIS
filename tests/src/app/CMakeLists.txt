#####################################################
# Don't forget to include output directory, otherwise
# the UI file won't be wrapped!
include_directories(
  ${CMAKE_SOURCE_DIR}/src/test
  ${CMAKE_CURRENT_BINARY_DIR}
)

#note for tests we should not include the moc of our
#qtests in the executable file list as the moc is
#directly included in the sources
#and should not be compiled twice. Trying to include
#them in will cause an error at build time

#No relinking and full RPATH for the install tree
#See: http://www.cmake.org/Wiki/CMake_RPATH_handling#No_relinking_and_full_RPATH_for_the_install_tree

macro (ADD_QGIS_TEST testname testsrc)
  set(qgis_${testname}_SRCS ${testsrc} ${util_SRCS})
  add_custom_target(qgis_${testname}moc ALL DEPENDS ${qgis_${testname}_MOC_SRCS})
  add_executable(qgis_${testname} ${qgis_${testname}_SRCS})
  target_compile_features(qgis_${testname} PRIVATE cxx_std_17)
  target_link_libraries(qgis_${testname}
    ${Qt5Core_LIBRARIES}
    ${Qt5Xml_LIBRARIES}
    ${Qt5Sql_LIBRARIES}
    ${Qt5Svg_LIBRARIES}
    ${Qt5Test_LIBRARIES}
    ${PROJ_LIBRARY}
    ${GEOS_LIBRARY}
    ${GDAL_LIBRARY}
    ${QWT_LIBRARY}
    qgis_core
    qgis_gui
    qgis_analysis
    qgis_app)
  if(WITH_QWTPOLAR AND NOT WITH_INTERNAL_QWTPOLAR)
    target_link_libraries(qgis_${testname} ${QWTPOLAR_LIBRARY})
  endif()
  if(WIN32)
    add_definitions(-DQWT_DLL)
    target_link_libraries(qgis_${testname} dbghelp)
  endif()
  if (APPLE)
    target_link_libraries(qgis_${testname} ${APP_SERVICES_LIBRARY} )
  endif()
  add_test(qgis_${testname} ${CMAKE_CURRENT_BINARY_DIR}/../../../output/bin/qgis_${testname} -maxwarnings 10000)
  #set_target_properties(qgis_${testname} PROPERTIES
  #  INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/${QGIS_LIB_DIR}
  #  INSTALL_RPATH_USE_LINK_PATH true )
endmacro (ADD_QGIS_TEST)

#############################################################
# Tests:

if (WITH_BINDINGS)
ADD_QGIS_TEST(apppythontest testqgisapppython.cpp)
endif()
ADD_QGIS_TEST(qgisapp testqgisapp.cpp)
ADD_QGIS_TEST(appbrowserproviders testqgsappbrowserproviders.cpp)
ADD_QGIS_TEST(qgisappclipboard testqgisappclipboard.cpp)
ADD_QGIS_TEST(appdockwidgets testqgisappdockwidgets.cpp)
ADD_QGIS_TEST(attributetabletest testqgsattributetable.cpp)
ADD_QGIS_TEST(applocatorfilters testqgsapplocatorfilters.cpp)
ADD_QGIS_TEST(decorationscalebar testqgsdecorationscalebar.cpp)
ADD_QGIS_TEST(fieldcalculatortest testqgsfieldcalculator.cpp)
ADD_QGIS_TEST(maptoolidentifyaction testqgsmaptoolidentifyaction.cpp)
ADD_QGIS_TEST(maptoollabel testqgsmaptoollabel.cpp)
ADD_QGIS_TEST(maptoolselect testqgsmaptoolselect.cpp)
ADD_QGIS_TEST(maptoolreshape testqgsmaptoolreshape.cpp)
ADD_QGIS_TEST(maptoolrotatefeature testqgsmaptoolrotatefeature.cpp)
ADD_QGIS_TEST(maptoolscalefeature testqgsmaptoolscalefeature.cpp)
ADD_QGIS_TEST(maptoolcircularstringtest testqgsmaptoolcircularstring.cpp)
ADD_QGIS_TEST(maptoolcircletest testqgsmaptoolcircle.cpp)
ADD_QGIS_TEST(maptoolmovefeaturetest testqgsmaptoolmovefeature.cpp)
ADD_QGIS_TEST(maptoolellipsetest testqgsmaptoolellipse.cpp)
ADD_QGIS_TEST(maptoolrectangletest testqgsmaptoolrectangle.cpp)
ADD_QGIS_TEST(maptoolregularpolygontest testqgsmaptoolregularpolygon.cpp)
ADD_QGIS_TEST(maptoolsplitpartstest testqgsmaptoolsplitparts.cpp)
ADD_QGIS_TEST(maptoolsplitfeaturestest testqgsmaptoolsplitfeatures.cpp)
ADD_QGIS_TEST(measuretool testqgsmeasuretool.cpp)
ADD_QGIS_TEST(vertextool testqgsvertextool.cpp)
ADD_QGIS_TEST(vectorlayersaveasdialogtest testqgsvectorlayersaveasdialog.cpp)
ADD_QGIS_TEST(maptoolreverselinetest testqgsmaptoolreverseline.cpp)
ADD_QGIS_TEST(maptooltrimextendfeaturetest testqgsmaptooltrimextendfeature.cpp)
ADD_QGIS_TEST(projectproperties testqgsprojectproperties.cpp)
ADD_QGIS_TEST(layoutvaliditychecks testqgsapplayoutvaliditychecks.cpp)
ADD_QGIS_TEST(meshcalculator testqgsmeshcalculatordialog.cpp)
ADD_QGIS_TEST(gpsinformationwidget testqgsgpsinformationwidget.cpp)
ADD_QGIS_TEST(labelpropertydialog testqgslabelpropertydialog.cpp)

add_subdirectory(maptooladdfeaturepoint)
add_subdirectory(maptooladdfeatureline)
