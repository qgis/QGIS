
SET (MEMORY_SRCS qgsmemoryprovider.cpp qgsmemoryfeatureiterator.cpp)

INCLUDE_DIRECTORIES(
  .
  ../../core
  ${GEOS_INCLUDE_DIR}
)

QT4_WRAP_CPP(MEMORY_MOC_SRCS qgsmemoryprovider.h)
ADD_LIBRARY(memoryprovider MODULE ${MEMORY_SRCS} ${MEMORY_MOC_SRCS})

TARGET_LINK_LIBRARIES(memoryprovider
  qgis_core
)


INSTALL (TARGETS memoryprovider
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})
