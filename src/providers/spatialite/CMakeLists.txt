
########################################################
# Files

SET(SPATIALITE_SRCS qgsspatialiteprovider.cpp)

SET(SPATIALITE_MOC_HDRS qgsspatialiteprovider.h)


########################################################
# Build

QT4_WRAP_CPP(SPATIALITE_MOC_SRCS ${SPATIALITE_MOC_HDRS})

INCLUDE_DIRECTORIES(
  ../../core
  ${SPATIALITE_INCLUDE_DIR}
  ${SPATIALITE_INCLUDE_DIR}
  ${GEOS_INCLUDE_DIR}
)

ADD_LIBRARY (spatialiteprovider MODULE ${SPATIALITE_SRCS} ${SPATIALITE_MOC_SRCS})

TARGET_LINK_LIBRARIES(spatialiteprovider
  ${QT_QTCORE_LIBRARY} 
  ${QT_QTXML_LIBRARY} 
  ${SPATIALITE_LIBRARY}
  qgis_core
)


########################################################
# Install
                
INSTALL(TARGETS spatialiteprovider
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})
