
########################################################
# Files

SET(OSM_SRCS
osmhandler.cpp
osmprovider.cpp
osmrenderer.cpp
osmstyle.cpp
)

SET(OSM_MOC_HDRS osmprovider.h)


########################################################
# Build

QT4_WRAP_CPP(OSM_MOC_SRCS ${OSM_MOC_HDRS})

INCLUDE_DIRECTORIES (
  ../../core
  ../../core/renderer
  ${GEOS_INCLUDE_DIR}
)

IF (WITH_INTERNAL_SPATIALITE)
  INCLUDE_DIRECTORIES(BEFORE ../../core/spatialite/headers/spatialite)
ELSE (WITH_INTERNAL_SPATIALITE)
  INCLUDE_DIRECTORIES(${SQLITE3_INCLUDE_DIR})
ENDIF (WITH_INTERNAL_SPATIALITE)

ADD_LIBRARY (osmprovider MODULE ${OSM_SRCS} ${OSM_MOC_SRCS})

TARGET_LINK_LIBRARIES (osmprovider
  qgis_core
)


########################################################
# Install

INSTALL(TARGETS osmprovider
  RUNTIME DESTINATION ${QGIS_PROVIDER_DIR}
  LIBRARY DESTINATION ${QGIS_PROVIDER_DIR})
