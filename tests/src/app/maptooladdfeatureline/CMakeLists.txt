#####################################################
# Don't forget to include output directory, otherwise
# the UI file won't be wrapped!
get_filename_component(PARENT_DIR ../ ABSOLUTE)

include_directories(
  ${PARENT_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
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

macro (ADD_QGIS_TEST TESTSRC)
  set (TESTNAME  ${TESTSRC})
  string(REPLACE "test" "" TESTNAME ${TESTNAME})
  string(REPLACE "qgs" "qgis_" TESTNAME ${TESTNAME})
  string(REPLACE ".cpp" "" TESTNAME ${TESTNAME})
  add_executable(${TESTNAME} ${TESTSRC} ${util_SRCS})
  target_compile_features(${TESTNAME} PRIVATE cxx_std_17)
  target_link_libraries(${TESTNAME}
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
  add_test(${TESTNAME} ${CMAKE_BINARY_DIR}/output/bin/${TESTNAME} -maxwarnings 10000)
  #set_target_properties(qgis_${testname} PROPERTIES
  #  INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/${QGIS_LIB_DIR}
  #  INSTALL_RPATH_USE_LINK_PATH true )
endmacro (ADD_QGIS_TEST)

#############################################################
# Tests:

set(TESTS
  testqgsmaptooladdfeatureline.cpp
  testqgsmaptooladdfeaturelinez.cpp
  testqgsmaptooladdfeaturelinem.cpp
  testqgsmaptooladdfeaturelinezm.cpp
  )

foreach(TESTSRC ${TESTS})
    ADD_QGIS_TEST(${TESTSRC})
endforeach(TESTSRC)
