
########################################################
# Files

set (wcs_SRCS
  qgswcs.cpp
  qgswcsutils.cpp
  qgswcsgetcapabilities.cpp
  qgswcsdescribecoverage.cpp
  qgswcsgetcoverage.cpp
)

########################################################
# Build

add_library (wcs MODULE ${wcs_SRCS})

# require c++17
target_compile_features(wcs PRIVATE cxx_std_17)

include_directories(
  ${CMAKE_SOURCE_DIR}/src/server
  ${CMAKE_SOURCE_DIR}/src/server/services
  ${CMAKE_SOURCE_DIR}/src/server/services/wcs

  ${CMAKE_BINARY_DIR}/src/python
  ${CMAKE_BINARY_DIR}/src/analysis
  ${CMAKE_BINARY_DIR}/src/server

  ${CMAKE_CURRENT_BINARY_DIR}

)


target_link_libraries(wcs
  qgis_core
  qgis_server
)


########################################################
# Install

install(TARGETS wcs
    RUNTIME DESTINATION ${QGIS_SERVER_MODULE_DIR}
    LIBRARY DESTINATION ${QGIS_SERVER_MODULE_DIR}
)

