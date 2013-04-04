########################################################
# Files

IF(WITH_QSPATIALITE)
  SUBDIRS(qspatialite)
ENDIF(WITH_QSPATIALITE)

SET(SPATIALITE_SRCS
  qgsspatialiteprovider.cpp
  qgsspatialitedataitems.cpp
  qgsspatialiteconnection.cpp
  qgsspatialitefeatureiterator.cpp
  qgsspatialitesourceselect.cpp
  qgsspatialitetablemodel.cpp
)

SET(SPATIALITE_MOC_HDRS
  qgsspatialiteprovider.h
  qgsspatialitedataitems.h
  qgsspatialitesourceselect.h
  qgsspatialitetablemodel.h
  qgsspatialiteconnection.h
)


########################################################
# Build

QT4_WRAP_CPP(SPATIALITE_MOC_SRCS ${SPATIALITE_MOC_HDRS})

IF(SPATIALITE_VERSION_GE_4_0_0)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DSPATIALITE_VERSION_GE_4_0_0")
ENDIF(SPATIALITE_VERSION_GE_4_0_0)


INCLUDE_DIRECTORIES(
  ../../core
  ../../gui
  ${CMAKE_CURRENT_BINARY_DIR}/../../ui
)

IF(WITH_INTERNAL_SPATIALITE)
  INCLUDE_DIRECTORIES(
    ../../core/spatialite/headers
    ../../core/spatialite/headers/spatialite
  )
ELSE(WITH_INTERNAL_SPATIALITE)
  INCLUDE_DIRECTORIES(${SPATIALITE_INCLUDE_DIR})
ENDIF(WITH_INTERNAL_SPATIALITE)

INCLUDE_DIRECTORIES(${GEOS_INCLUDE_DIR})

ADD_LIBRARY (spatialiteprovider MODULE ${SPATIALITE_SRCS} ${SPATIALITE_MOC_SRCS})

TARGET_LINK_LIBRARIES(spatialiteprovider
  qgis_core
  qgis_gui
)

IF(NOT WITH_INTERNAL_SPATIALITE)
  TARGET_LINK_LIBRARIES(spatialiteprovider ${SPATIALITE_LIBRARY})
ENDIF(NOT WITH_INTERNAL_SPATIALITE)


########################################################
# Install

INSTALL(TARGETS spatialiteprovider
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})
