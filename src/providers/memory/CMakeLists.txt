
SET (MEMORY_SRCS qgsmemoryprovider.cpp)

INCLUDE_DIRECTORIES(
  .
  ../../core ../../core/spatialindex
  ${GEOS_INCLUDE_DIR}
)

ADD_LIBRARY(memoryprovider MODULE ${MEMORY_SRCS})

TARGET_LINK_LIBRARIES(memoryprovider
  qgis_core
)


INSTALL (TARGETS memoryprovider
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})
