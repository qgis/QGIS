
########################################################
# Files

set (GPX_SRCS
  qgsgpxprovider.cpp
  qgsgpxfeatureiterator.cpp
  gpsdata.cpp
)

########################################################
# Build

include_directories(
  ${CMAKE_SOURCE_DIR}/src/core
  ${CMAKE_SOURCE_DIR}/src/core/geometry
  ${CMAKE_SOURCE_DIR}/src/core/metadata
  ${CMAKE_SOURCE_DIR}/src/core/expression
  ${CMAKE_SOURCE_DIR}/src/core/symbology
  ${CMAKE_SOURCE_DIR}/external
  ${CMAKE_SOURCE_DIR}/external/nlohmann

  ${CMAKE_BINARY_DIR}/src/core
  ${CMAKE_BINARY_DIR}/src/gui
)
include_directories (SYSTEM
  ${EXPAT_INCLUDE_DIR}
)

add_library(gpxprovider MODULE ${GPX_SRCS})

target_link_libraries(gpxprovider
  ${EXPAT_LIBRARY}
  qgis_core
)


########################################################
# Install

install (TARGETS gpxprovider
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})
