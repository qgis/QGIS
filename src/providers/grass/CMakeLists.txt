
ADD_DEFINITIONS(-DGRASS_BASE=\\\"${GRASS_PREFIX}\\\")

########################################################
# Files

SET(GRASS_PROVIDER_SRCS provider.cpp)

SET(GRASS_LIB_SRCS qgsgrassprovider.cpp qgsgrass.cpp)


########################################################
# Build

INCLUDE_DIRECTORIES (
  ../../core
  ${GRASS_INCLUDE_DIR}
  ${GDAL_INCLUDE_DIR}
  ${PROJ_INCLUDE_DIR}
)

ADD_LIBRARY (qgisgrass SHARED ${GRASS_LIB_SRCS})

TARGET_LINK_LIBRARIES (qgisgrass
  ${QT_LIBRARIES}
  ${GRASS_LIBRARIES}
  qgis_core
)

ADD_LIBRARY (grassprovider SHARED ${GRASS_PROVIDER_SRCS})

TARGET_LINK_LIBRARIES (grassprovider
  ${QT_LIBRARIES}
  qgis_core
  qgisgrass
)

########################################################
# Install

INSTALL(TARGETS qgisgrass
  RUNTIME DESTINATION ${QGIS_BIN_DIR}
  LIBRARY DESTINATION lib)
       
INSTALL(TARGETS grassprovider
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})
