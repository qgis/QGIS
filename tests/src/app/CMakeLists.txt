#####################################################
# Don't forget to include output directory, otherwise
# the UI file won't be wrapped!
INCLUDE_DIRECTORIES(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/src/core
  ${CMAKE_SOURCE_DIR}/src/core/auth
  ${CMAKE_SOURCE_DIR}/src/core/expression
  ${CMAKE_SOURCE_DIR}/src/core/geometry
  ${CMAKE_SOURCE_DIR}/src/core/layout
  ${CMAKE_SOURCE_DIR}/src/core/locator
  ${CMAKE_SOURCE_DIR}/src/core/metadata
  ${CMAKE_SOURCE_DIR}/src/core/mesh
  ${CMAKE_SOURCE_DIR}/src/core/raster
  ${CMAKE_SOURCE_DIR}/src/core/symbology
  ${CMAKE_SOURCE_DIR}/src/core/validity
  ${CMAKE_SOURCE_DIR}/src/ui
  ${CMAKE_SOURCE_DIR}/src/gui
  ${CMAKE_SOURCE_DIR}/src/gui/editorwidgets
  ${CMAKE_SOURCE_DIR}/src/gui/editorwidgets/core
  ${CMAKE_SOURCE_DIR}/src/gui/attributetable
  ${CMAKE_SOURCE_DIR}/src/gui/symbology
  ${CMAKE_SOURCE_DIR}/src/gui/raster
  ${CMAKE_SOURCE_DIR}/src/python
  ${CMAKE_SOURCE_DIR}/src/app
  ${CMAKE_SOURCE_DIR}/src/app/mesh
  ${CMAKE_SOURCE_DIR}/src/app/pluginmanager
  ${CMAKE_SOURCE_DIR}/src/test
  ${CMAKE_SOURCE_DIR}/src/analysis
  ${CMAKE_SOURCE_DIR}/src/analysis/mesh

  ${CMAKE_BINARY_DIR}/src/analysis
  ${CMAKE_BINARY_DIR}/src/core
  ${CMAKE_BINARY_DIR}/src/gui
  ${CMAKE_BINARY_DIR}/src/python
  ${CMAKE_BINARY_DIR}/src/app
  ${CMAKE_BINARY_DIR}/src/ui
  ${CMAKE_CURRENT_BINARY_DIR}
)
INCLUDE_DIRECTORIES(SYSTEM
  ${QT_INCLUDE_DIR}
  ${QWT_INCLUDE_DIR}
  ${GDAL_INCLUDE_DIR}
  ${GEOS_INCLUDE_DIR}
  ${QCA_INCLUDE_DIR}
  ${QTKEYCHAIN_INCLUDE_DIR}
  ${QSCINTILLA_INCLUDE_DIR}
)


#note for tests we should not include the moc of our
#qtests in the executable file list as the moc is
#directly included in the sources
#and should not be compiled twice. Trying to include
#them in will cause an error at build time

#No relinking and full RPATH for the install tree
#See: http://www.cmake.org/Wiki/CMake_RPATH_handling#No_relinking_and_full_RPATH_for_the_install_tree

MACRO (ADD_QGIS_TEST testname testsrc)
  SET(qgis_${testname}_SRCS ${testsrc} ${util_SRCS})
  ADD_CUSTOM_TARGET(qgis_${testname}moc ALL DEPENDS ${qgis_${testname}_MOC_SRCS})
  ADD_EXECUTABLE(qgis_${testname} ${qgis_${testname}_SRCS})
  SET_TARGET_PROPERTIES(qgis_${testname} PROPERTIES AUTOMOC TRUE)
  TARGET_LINK_LIBRARIES(qgis_${testname}
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
  IF(WITH_QWTPOLAR AND NOT WITH_INTERNAL_QWTPOLAR)
    TARGET_LINK_LIBRARIES(qgis_${testname} ${QWTPOLAR_LIBRARY})
  ENDIF(WITH_QWTPOLAR AND NOT WITH_INTERNAL_QWTPOLAR)
  IF(WIN32)
    ADD_DEFINITIONS(-DQWT_DLL)
    TARGET_LINK_LIBRARIES(qgis_${testname} DbgHelp)
  ENDIF(WIN32)
  IF (APPLE)
    TARGET_LINK_LIBRARIES(qgis_${testname} ${APP_SERVICES_LIBRARY} )
  ENDIF(APPLE)
  ADD_TEST(qgis_${testname} ${CMAKE_CURRENT_BINARY_DIR}/../../../output/bin/qgis_${testname} -maxwarnings 10000)
  #SET_TARGET_PROPERTIES(qgis_${testname} PROPERTIES
  #  INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/${QGIS_LIB_DIR}
  #  INSTALL_RPATH_USE_LINK_PATH true )
ENDMACRO (ADD_QGIS_TEST)

#############################################################
# Tests:

IF (WITH_BINDINGS)
ADD_QGIS_TEST(apppythontest testqgisapppython.cpp)
ENDIF ()
ADD_QGIS_TEST(qgisapp testqgisapp.cpp)
ADD_QGIS_TEST(appbrowserproviders testqgsappbrowserproviders.cpp)
ADD_QGIS_TEST(qgisappclipboard testqgisappclipboard.cpp)
ADD_QGIS_TEST(attributetabletest testqgsattributetable.cpp)
ADD_QGIS_TEST(applocatorfilters testqgsapplocatorfilters.cpp)
ADD_QGIS_TEST(fieldcalculatortest testqgsfieldcalculator.cpp)
ADD_QGIS_TEST(maptooladdfeatureline testqgsmaptooladdfeatureline.cpp)
ADD_QGIS_TEST(maptooladdfeaturepoint testqgsmaptooladdfeaturepoint.cpp)
ADD_QGIS_TEST(maptoolidentifyaction testqgsmaptoolidentifyaction.cpp)
ADD_QGIS_TEST(maptoolselect testqgsmaptoolselect.cpp)
ADD_QGIS_TEST(maptoolreshape testqgsmaptoolreshape.cpp)
ADD_QGIS_TEST(maptoolcircularstringtest testqgsmaptoolcircularstring.cpp)
ADD_QGIS_TEST(maptoolcircletest testqgsmaptoolcircle.cpp)
ADD_QGIS_TEST(maptoolellipsetest testqgsmaptoolellipse.cpp)
ADD_QGIS_TEST(maptoolrectangletest testqgsmaptoolrectangle.cpp)
ADD_QGIS_TEST(maptoolregularpolygontest testqgsmaptoolregularpolygon.cpp)
ADD_QGIS_TEST(measuretool testqgsmeasuretool.cpp)
ADD_QGIS_TEST(vertextool testqgsvertextool.cpp)
ADD_QGIS_TEST(vectorlayersaveasdialogtest testqgsvectorlayersaveasdialog.cpp)
ADD_QGIS_TEST(maptoolreverselinetest testqgsmaptoolreverseline.cpp)
ADD_QGIS_TEST(maptooltrimextendfeaturetest testqgsmaptooltrimextendfeature.cpp)
ADD_QGIS_TEST(projectproperties testqgsprojectproperties.cpp)
ADD_QGIS_TEST(layoutvaliditychecks testqgsapplayoutvaliditychecks.cpp)
ADD_QGIS_TEST(meshcalculator testqgsmeshcalculatordialog.cpp)
ADD_QGIS_TEST(meshlayerpropertiesdialog testqgsmeshlayerpropertiesdialog.cpp)
