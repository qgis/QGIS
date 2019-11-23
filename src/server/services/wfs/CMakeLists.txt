
########################################################
# Files

SET (WFS_SRCS
  qgswfs.cpp
  qgswfsutils.cpp
  qgswfsgetcapabilities.cpp
  qgswfsgetcapabilities_1_0_0.cpp
  qgswfsdescribefeaturetype.cpp
  qgswfsgetfeature.cpp
  qgswfstransaction.cpp
  qgswfstransaction_1_0_0.cpp
  qgswfsparameters.cpp
)

SET (WFS_HDRS
  qgswfsparameters.h
)

########################################################
# Build

ADD_LIBRARY (wfs MODULE ${WFS_SRCS} ${WFS_HDRS})


INCLUDE_DIRECTORIES(SYSTEM
  ${GDAL_INCLUDE_DIR}
  ${POSTGRES_INCLUDE_DIR}
)

INCLUDE_DIRECTORIES(
  ${CMAKE_SOURCE_DIR}/external
  ${CMAKE_SOURCE_DIR}/external/nlohmann
  ${CMAKE_SOURCE_DIR}/src/core
  ${CMAKE_SOURCE_DIR}/src/core/dxf
  ${CMAKE_SOURCE_DIR}/src/core/expression
  ${CMAKE_SOURCE_DIR}/src/core/geometry
  ${CMAKE_SOURCE_DIR}/src/core/metadata
  ${CMAKE_SOURCE_DIR}/src/core/raster
  ${CMAKE_SOURCE_DIR}/src/core/symbology
  ${CMAKE_SOURCE_DIR}/src/core/effects
  ${CMAKE_SOURCE_DIR}/src/core/layertree
  ${CMAKE_SOURCE_DIR}/src/core/fieldformatter
  ${CMAKE_SOURCE_DIR}/src/server
  ${CMAKE_SOURCE_DIR}/src/server/services
  ${CMAKE_SOURCE_DIR}/src/server/services/wfs

  ${CMAKE_BINARY_DIR}/src/core
  ${CMAKE_BINARY_DIR}/src/python
  ${CMAKE_BINARY_DIR}/src/analysis
  ${CMAKE_BINARY_DIR}/src/server

  ${CMAKE_CURRENT_BINARY_DIR}
)


TARGET_LINK_LIBRARIES(wfs
  qgis_core
  qgis_server
)


########################################################
# Install

INSTALL(TARGETS wfs
    RUNTIME DESTINATION ${QGIS_SERVER_MODULE_DIR}
    LIBRARY DESTINATION ${QGIS_SERVER_MODULE_DIR}
)

