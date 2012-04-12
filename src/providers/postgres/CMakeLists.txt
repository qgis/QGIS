
########################################################
# Files

SET(PG_SRCS
  qgspostgresprovider.cpp
  qgspostgresconn.cpp
  qgspostgresdataitems.cpp
  qgspgsourceselect.cpp
  qgspgnewconnection.cpp
  qgspgtablemodel.cpp
  qgscolumntypethread.cpp
)
SET(PG_MOC_HDRS
  qgspostgresprovider.h
  qgspostgresconn.h
  qgspostgresdataitems.h
  qgspgsourceselect.h
  qgspgnewconnection.h
  qgspgtablemodel.h
  qgscolumntypethread.h
)


########################################################
# Build

QT4_WRAP_CPP(PG_MOC_SRCS ${PG_MOC_HDRS})

INCLUDE_DIRECTORIES(
  ../../core
  ${POSTGRES_INCLUDE_DIR}
  ${GEOS_INCLUDE_DIR}
  ../../core
  ../../gui
  ${CMAKE_CURRENT_BINARY_DIR}/../../ui
)

ADD_LIBRARY (postgresprovider MODULE ${PG_SRCS} ${PG_MOC_SRCS})

TARGET_LINK_LIBRARIES (postgresprovider
  ${POSTGRES_LIBRARY}
  qgis_core
  qgis_gui
)


########################################################
# Install
                
INSTALL(TARGETS postgresprovider
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})
