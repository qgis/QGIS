########################################################
# Files

ADD_SUBDIRECTORY(ocispatial)

SET(ORACLE_SRCS
  qgsoracleprovider.cpp
  qgsoracleconn.cpp
  qgsoracledataitems.cpp
  qgsoraclesourceselect.cpp
  qgsoraclenewconnection.cpp
  qgsoracletablecache.cpp
  qgsoracletablemodel.cpp
  qgsoraclecolumntypethread.cpp
  qgsoraclefeatureiterator.cpp
)

SET(ORACLE_MOC_HDRS
  qgsoracleprovider.h
  qgsoracleconn.h
  qgsoracledataitems.h
  qgsoraclesourceselect.h
  qgsoraclenewconnection.h
  qgsoracletablemodel.h
  qgsoraclecolumntypethread.h
)


########################################################
# Build

QT4_WRAP_CPP(ORACLE_MOC_SRCS ${ORACLE_MOC_HDRS})

INCLUDE_DIRECTORIES(
  ../../core
  ../../core/geometry
  ../../gui
  ${GEOS_INCLUDE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}/../../ui
  ${QT_QTSQL_INCLUDEDIR}
  ${QSCINTILLA_INCLUDE_DIR}
)

ADD_LIBRARY (oracleprovider MODULE ${ORACLE_SRCS} ${ORACLE_MOC_SRCS})

TARGET_LINK_LIBRARIES (oracleprovider
  qgis_core
  qgis_gui
  ${QT_QTSQL_LIBRARY}
)


########################################################
# Install

INSTALL(TARGETS oracleprovider
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})
