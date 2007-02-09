
########################################################
# Files

SET(PG_SRCS
qgspostgresprovider.cpp
qgspostgisbox2d.cpp
qgspostgisbox3d.cpp
qgspostgrescountthread.cpp
qgspostgresextentthread.cpp
)

IF(NOT WIN32)
    SET(PG_MOC_HDRS qgspostgresprovider.h)
ENDIF(NOT WIN32)

########################################################
# Build

QT4_WRAP_CPP(PG_MOC_SRCS ${PG_MOC_HDRS})

INCLUDE_DIRECTORIES (
  ../../core
  ../../gui
  ${CMAKE_CURRENT_BINARY_DIR}/../../ui
  ${POSTGRES_INCLUDE_DIR}
  $GEOS_INCLUDE_DIR}
)

ADD_LIBRARY (postgresprovider MODULE ${PG_SRCS} ${PG_MOC_SRCS})

TARGET_LINK_LIBRARIES (postgresprovider
  ${QT_LIBRARIES}
  ${POSTGRES_LIBRARY}
  qgis_core
)


########################################################
# Install
                
INSTALL(TARGETS postgresprovider
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})
