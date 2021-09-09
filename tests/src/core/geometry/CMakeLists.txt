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
  testqgscircle.cpp
  testqgscircularstring.cpp
  testqgscompoundcurve.cpp
  testqgscurvepolygon.cpp
  testqgsellipse.cpp
  testqgsgeometry.cpp
  testqgsgeometrycollection.cpp
  testqgsgeometryutils.cpp
  testqgslinestring.cpp
  testqgsmulticurve.cpp
  testqgsmultilinestring.cpp
  testqgsmultipoint.cpp
  testqgsmultipolygon.cpp
  testqgsmultisurface.cpp
  testqgspoint.cpp
  testqgspointxy.cpp
  testqgspolygon.cpp
  testqgsquadrilateral.cpp
  testqgsrectangle.cpp
  testqgsregularpolygon.cpp
  testqgstriangle.cpp
  )

foreach(TESTSRC ${TESTS})
  add_qgis_test(${TESTSRC} MODULE core LINKEDLIBRARIES qgis_core)
endforeach(TESTSRC)
