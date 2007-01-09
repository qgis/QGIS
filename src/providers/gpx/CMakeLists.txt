
########################################################
# Files

SET (OGR_SRCS qgsgpxprovider.cpp gpsdata.cpp)


########################################################
# Build

INCLUDE_DIRECTORIES(
  ../../core
  ${GEOS_INCLUDE_DIR}
  ${EXPAT_INCLUDE_DIR}
)

ADD_LIBRARY(gpxprovider SHARED ${OGR_SRCS})

TARGET_LINK_LIBRARIES(gpxprovider
  ${QT_LIBRARIES}
  ${GEOS_LIBRARY}
  ${EXPAT_LIBRARY}
  qgis_core
)


########################################################
# Install

INSTALL (TARGETS gpxprovider
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR}) 
