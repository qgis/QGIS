########################################################
# Files

IF(WITH_QSPATIALITE)
  SUBDIRS(qspatialite)
ENDIF(WITH_QSPATIALITE)

SET(SPATIALITE_SRCS
  qgsspatialiteprovider.cpp
  qgsspatialitedataitems.cpp
  qgsspatialiteconnection.cpp
  qgsspatialiteconnpool.cpp
  qgsspatialitefeatureiterator.cpp
  qgsspatialitesourceselect.cpp
  qgsspatialitetablemodel.cpp
)

SET(SPATIALITE_MOC_HDRS
  qgsspatialiteprovider.h
  qgsspatialiteconnpool.h
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
IF(SPATIALITE_VERSION_G_4_1_1)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DSPATIALITE_VERSION_G_4_1_1")
ENDIF(SPATIALITE_VERSION_G_4_1_1)


INCLUDE_DIRECTORIES(
  ${QSCINTILLA_INCLUDE_DIR}
  ../../core
  ../../gui
  ${CMAKE_CURRENT_BINARY_DIR}/../../ui
  ${SPATIALITE_INCLUDE_DIR}
  ${GEOS_INCLUDE_DIR}
)

ADD_LIBRARY (spatialiteprovider MODULE ${SPATIALITE_SRCS} ${SPATIALITE_MOC_SRCS})

TARGET_LINK_LIBRARIES(spatialiteprovider
  qgis_core
  qgis_gui
  ${SPATIALITE_LIBRARY}
)

########################################################
# Install

INSTALL(TARGETS spatialiteprovider
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})
