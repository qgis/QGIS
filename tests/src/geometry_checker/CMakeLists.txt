# Standard includes and utils to compile into all tests.

#####################################################
# Don't forget to include output directory, otherwise
# the UI file won't be wrapped!
include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/src/test
  ${CMAKE_SOURCE_DIR}/src/plugins

  ${CMAKE_BINARY_DIR}/src/analysis
)
include_directories(SYSTEM
  ${POSTGRES_INCLUDE_DIR}
)

#############################################################
# Tests:

add_qgis_test(testqgsgeometrychecks.cpp MODULE geometry_checker LINKEDLIBRARIES qgis_analysis)
add_qgis_test(testqgsvectorlayerfeaturepool.cpp MODULE geometry_checker LINKEDLIBRARIES qgis_analysis)

