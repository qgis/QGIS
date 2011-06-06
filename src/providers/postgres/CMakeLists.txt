
########################################################
# Files

SET(PG_SRCS
  qgspostgresprovider.cpp
  qgspostgresconnection.cpp
)
SET(PG_MOC_HDRS
  qgspostgresprovider.h
  qgspostgresconnection.h
)


########################################################
# Build

QT4_WRAP_CPP(PG_MOC_SRCS ${PG_MOC_HDRS})

INCLUDE_DIRECTORIES(
  ../../core
  ${POSTGRES_INCLUDE_DIR}
  ${GEOS_INCLUDE_DIR}
)

ADD_LIBRARY (postgresprovider MODULE ${PG_SRCS} ${PG_MOC_SRCS})

TARGET_LINK_LIBRARIES (postgresprovider
  ${POSTGRES_LIBRARY}
  qgis_core
)


########################################################
# Install
                
INSTALL(TARGETS postgresprovider
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})
