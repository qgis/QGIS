########################################################
# Files

ADD_SUBDIRECTORY(ocispatial)

SET(ORACLE_SRCS
  qgsoracleprovider.cpp
  qgsoracleconn.cpp
  qgsoracledataitems.cpp
  qgsoraclenewconnection.cpp
  qgsoracletablecache.cpp
  qgsoracletablemodel.cpp
  qgsoraclecolumntypethread.cpp
  qgsoraclefeatureiterator.cpp
  qgsoracleconnpool.cpp
  qgsoracleexpressioncompiler.cpp
  qgsoracletransaction.cpp
)

IF (WITH_GUI)
  SET(ORACLE_SRCS ${ORACLE_SRCS}
    qgsoraclesourceselect.cpp
  )
ENDIF ()

SET(ORACLE_HDRS
  qgsoracleexpressioncompiler.h
)


########################################################
# Build

INCLUDE_DIRECTORIES(
  ${CMAKE_SOURCE_DIR}/src/core
  ${CMAKE_SOURCE_DIR}/src/core/auth
  ${CMAKE_SOURCE_DIR}/src/core/expression
  ${CMAKE_SOURCE_DIR}/src/core/geometry
  ${CMAKE_SOURCE_DIR}/src/core/metadata
  ${CMAKE_SOURCE_DIR}/src/core/symbology
  ${CMAKE_SOURCE_DIR}/src/gui
  ${CMAKE_SOURCE_DIR}/src/gui/auth
  ${CMAKE_SOURCE_DIR}/external
  ${CMAKE_SOURCE_DIR}/external/nlohmann

  ${CMAKE_BINARY_DIR}/src/core
  ${CMAKE_BINARY_DIR}/src/gui
  ${CMAKE_BINARY_DIR}/src/ui
)
INCLUDE_DIRECTORIES(SYSTEM
  ${GEOS_INCLUDE_DIR}
  ${QT_QTSQL_INCLUDEDIR}
)

IF (WITH_GUI)
  INCLUDE_DIRECTORIES(SYSTEM
    ${QSCINTILLA_INCLUDE_DIR}
  )
ENDIF ()

ADD_LIBRARY (oracleprovider MODULE ${ORACLE_SRCS} ${ORACLE_HDRS})

TARGET_LINK_LIBRARIES (oracleprovider
  qgis_core
  ${Qt5Sql_LIBRARIES}
)

IF (WITH_GUI)
  TARGET_LINK_LIBRARIES (oracleprovider
    qgis_gui
  )
  ADD_DEPENDENCIES(oracleprovider ui)
ENDIF ()


########################################################
# Install

INSTALL(TARGETS oracleprovider
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})
