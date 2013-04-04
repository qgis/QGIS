
########################################################
# Files

SET (GPX_SRCS
  qgsgpxprovider.cpp
  qgsgpxfeatureiterator.cpp
  gpsdata.cpp
)

SET (GPX_MOC_HDRS
	qgsgpxprovider.h
)

########################################################
# Build

INCLUDE_DIRECTORIES(
  ../../core
  ${GEOS_INCLUDE_DIR}
  ${EXPAT_INCLUDE_DIR}
)

QT4_WRAP_CPP(GPX_MOC_SRCS ${GPX_MOC_HDRS})

ADD_LIBRARY(gpxprovider MODULE ${GPX_SRCS} ${GPX_MOC_SRCS})

TARGET_LINK_LIBRARIES(gpxprovider
  ${EXPAT_LIBRARY}
  qgis_core
)


########################################################
# Install

INSTALL (TARGETS gpxprovider
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR}) 
