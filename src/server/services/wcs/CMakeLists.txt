
########################################################
# Files

SET (wcs_SRCS
  qgswcs.cpp
  qgswcsutils.cpp
  qgswcsgetcapabilities.cpp
  qgswcsdescribecoverage.cpp
  qgswcsgetcoverage.cpp
)

########################################################
# Build

ADD_LIBRARY (wcs MODULE ${wcs_SRCS})


INCLUDE_DIRECTORIES(SYSTEM
  ${GDAL_INCLUDE_DIR}
  ${POSTGRES_INCLUDE_DIR}
)

INCLUDE_DIRECTORIES(
  ${CMAKE_BINARY_DIR}/src/core
  ${CMAKE_BINARY_DIR}/src/python
  ${CMAKE_BINARY_DIR}/src/analysis
  ${CMAKE_BINARY_DIR}/src/server
  ${CMAKE_CURRENT_BINARY_DIR}
  ../../../core
  ../../../core/dxf
  ../../../core/expression
  ../../../core/geometry
  ../../../core/metadata
  ../../../core/raster
  ../../../core/symbology
  ../../../core/layertree
  ../..
  ..
  .
)


TARGET_LINK_LIBRARIES(wcs
  qgis_core
  qgis_server
)


########################################################
# Install

INSTALL(TARGETS wcs
    RUNTIME DESTINATION ${QGIS_SERVER_MODULE_DIR}
    LIBRARY DESTINATION ${QGIS_SERVER_MODULE_DIR}
)

