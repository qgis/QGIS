########################################################
# Files

SET (wfs3_SRCS
  ${CMAKE_SOURCE_DIR}/external/nlohmann/json.hpp
  ${CMAKE_SOURCE_DIR}/external/inja/inja.hpp
  qgswfs3.cpp
)

SET (wfs3_MOC_HDRS
)

########################################################
# Build

QT5_WRAP_CPP(wfs3_MOC_SRCS ${wfs3_MOC_HDRS})

ADD_LIBRARY (wfs3 MODULE ${wfs3_SRCS} ${wfs3_MOC_SRCS} ${wfs3_MOC_HDRS})


INCLUDE_DIRECTORIES(SYSTEM
  ${GDAL_INCLUDE_DIR}
  ${POSTGRES_INCLUDE_DIR}
)

INCLUDE_DIRECTORIES(
  ${CMAKE_SOURCE_DIR}/external
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
  ../../../core/fieldformatter
  ../..
  ..
  .
)


TARGET_LINK_LIBRARIES(wfs3
  qgis_core
  qgis_server
)


########################################################
# Install

INSTALL(TARGETS wfs3
    RUNTIME DESTINATION ${QGIS_SERVER_MODULE_DIR}
    LIBRARY DESTINATION ${QGIS_SERVER_MODULE_DIR}
)

