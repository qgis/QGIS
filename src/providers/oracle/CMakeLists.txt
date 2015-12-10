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
  qgsoracleconnpool.cpp
  qgsoracleexpressioncompiler.cpp
)

SET(ORACLE_MOC_HDRS
  qgsoracleprovider.h
  qgsoracleconn.h
  qgsoracledataitems.h
  qgsoraclesourceselect.h
  qgsoraclenewconnection.h
  qgsoracletablemodel.h
  qgsoraclecolumntypethread.h
  qgsoracleconnpool.h
)

SET(ORACLE_HDRS
  qgsoracleexpressioncompiler.h
)


########################################################
# Build

QT4_WRAP_CPP(ORACLE_MOC_SRCS ${ORACLE_MOC_HDRS})

INCLUDE_DIRECTORIES(
  ../../core
  ../../core/geometry
  ../../gui
)
INCLUDE_DIRECTORIES(SYSTEM
  ${GEOS_INCLUDE_DIR}
)
INCLUDE_DIRECTORIES(
  ${CMAKE_CURRENT_BINARY_DIR}/../../ui
)
INCLUDE_DIRECTORIES(SYSTEM
  ${QT_QTSQL_INCLUDEDIR}
  ${QSCINTILLA_INCLUDE_DIR}
)

ADD_LIBRARY (oracleprovider MODULE ${ORACLE_SRCS} ${ORACLE_HDRS} ${ORACLE_MOC_SRCS})

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
