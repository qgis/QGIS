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

#############################################################
# Tests:

set(TESTS
  testqgsgeometry.cpp
  testqgsgeometryutils.cpp
  testqgspoint.cpp
  testqgspointxy.cpp
  testqgspolygon.cpp
  testqgsmultipoint.cpp
  testqgslinestring.cpp
  testqgsrectangle.cpp
  )

foreach(TESTSRC ${TESTS})
  add_qgis_test(${TESTSRC} MODULE app LINKEDLIBRARIES qgis_app)
endforeach(TESTSRC)
