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
)

SET(ORACLE_MOC_HDRS
  qgsoracleprovider.h
  qgsoracleconn.h
  qgsoracledataitems.h
  qgsoraclenewconnection.h
  qgsoracletablemodel.h
  qgsoraclecolumntypethread.h
  qgsoracleconnpool.h
)

IF (WITH_GUI)
  SET(ORACLE_SRCS ${ORACLE_SRCS}
    qgsoraclesourceselect.cpp
  )
  SET(ORACLE_MOC_HDRS ${ORACLE_MOC_HDRS}
    qgsoraclesourceselect.h
  )
ENDIF ()

SET(ORACLE_HDRS
  qgsoracleexpressioncompiler.h
)


########################################################
# Build

QT5_WRAP_CPP(ORACLE_MOC_SRCS ${ORACLE_MOC_HDRS})

INCLUDE_DIRECTORIES(
  ${CMAKE_SOURCE_DIR}/src/core
  ${CMAKE_SOURCE_DIR}/src/core/expression
  ${CMAKE_SOURCE_DIR}/src/core/geometry
  ${CMAKE_SOURCE_DIR}/src/core/metadata
  ${CMAKE_SOURCE_DIR}/src/gui

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

ADD_LIBRARY (oracleprovider MODULE ${ORACLE_SRCS} ${ORACLE_HDRS} ${ORACLE_MOC_SRCS})

TARGET_LINK_LIBRARIES (oracleprovider
  qgis_core
  ${Qt5Sql_LIBRARIES}
)

IF (WITH_GUI)
  TARGET_LINK_LIBRARIES (oracleprovider
    qgis_gui
  )
ENDIF ()


########################################################
# Install

INSTALL(TARGETS oracleprovider
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})
