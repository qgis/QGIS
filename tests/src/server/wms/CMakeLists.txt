#####################################################
# Don't forget to include output directory, otherwise
# the UI file won't be wrapped!
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/src/core
  ${CMAKE_SOURCE_DIR}/src/core/geometry
  ${CMAKE_SOURCE_DIR}/src/core/expression
  ${CMAKE_SOURCE_DIR}/src/core/dxf
  ${CMAKE_SOURCE_DIR}/src/core/symbology
  ${CMAKE_SOURCE_DIR}/src/core/metadata
  ${CMAKE_SOURCE_DIR}/src/core/layertree
  ${CMAKE_SOURCE_DIR}/src/core/raster
  ${CMAKE_SOURCE_DIR}/src/core/annotations
  ${CMAKE_SOURCE_DIR}/src/core/layout
  ${CMAKE_BINARY_DIR}/src/core
  ${CMAKE_SOURCE_DIR}/src/test
  ${CMAKE_SOURCE_DIR}/src/server
  ${CMAKE_BINARY_DIR}/src/server
  ${CMAKE_SOURCE_DIR}/src/server/services
  ${CMAKE_SOURCE_DIR}/src/server/services/wms
)

#note for tests we should not include the moc of our
#qtests in the executable file list as the moc is
#directly included in the sources
#and should not be compiled twice. Trying to include
#them in will cause an error at build time

#No relinking and full RPATH for the install tree
#See: http://www.cmake.org/Wiki/CMake_RPATH_handling#No_relinking_and_full_RPATH_for_the_install_tree
SET(MODULE_WMS_SRCS
  ${CMAKE_SOURCE_DIR}/src/server/services/wms/qgswmsrenderer.cpp
  ${CMAKE_SOURCE_DIR}/src/server/services/wms/qgslayerrestorer.cpp
  ${CMAKE_SOURCE_DIR}/src/server/services/wms/qgsmaprendererjobproxy.cpp
  ${CMAKE_SOURCE_DIR}/src/server/services/wms/qgswmsparameters.cpp
)

MACRO (ADD_QGIS_TEST TESTSRC)
  SET (TESTNAME  ${TESTSRC})
  STRING(REPLACE "test" "" TESTNAME ${TESTNAME})
  STRING(REPLACE "qgs" "" TESTNAME ${TESTNAME})
  STRING(REPLACE ".cpp" "" TESTNAME ${TESTNAME})
  SET (TESTNAME  "qgis_${TESTNAME}test")
  ADD_EXECUTABLE(${TESTNAME} ${TESTSRC} ${MODULE_WMS_SRCS})
  SET_TARGET_PROPERTIES(${TESTNAME} PROPERTIES AUTOMOC TRUE)
  TARGET_LINK_LIBRARIES(${TESTNAME}
    ${Qt5Core_LIBRARIES}
    ${Qt5Xml_LIBRARIES}
    ${Qt5Svg_LIBRARIES}
    ${Qt5Test_LIBRARIES}
    ${PROJ_LIBRARY}
    ${GEOS_LIBRARY}
    ${GDAL_LIBRARY}
    qgis_core
    qgis_server
  )
  ADD_TEST(${TESTNAME} ${CMAKE_BINARY_DIR}/output/bin/${TESTNAME} -maxwarnings 10000)
ENDMACRO (ADD_QGIS_TEST)

#############################################################
# Tests:

SET(TESTS
  test_qgsserver_wms_dxf.cpp
)

FOREACH(TESTSRC ${TESTS})
    ADD_QGIS_TEST(${TESTSRC})
ENDFOREACH(TESTSRC)
