SET(MSSQL_SRCS
  qgsmssqlconnection.cpp
  qgsmssqlprovider.cpp
  qgsmssqlgeometryparser.cpp
  qgsmssqltablemodel.cpp
  qgsmssqldataitems.cpp
  qgsmssqlexpressioncompiler.cpp
  qgsmssqlfeatureiterator.cpp
  qgsmssqlgeomcolumntypethread.cpp
)

SET(MSSQL_MOC_HDRS
  qgsmssqlprovider.h
  qgsmssqltablemodel.h
  qgsmssqldataitems.h
  qgsmssqlgeomcolumntypethread.h
)

IF (WITH_GUI)
  SET(MSSQL_SRCS ${MSSQL_SRCS}
    qgsmssqlsourceselect.cpp
    qgsmssqlnewconnection.cpp
  )
  SET(MSSQL_MOC_HDRS ${MSSQL_MOC_HDRS}
    qgsmssqlsourceselect.h
    qgsmssqlnewconnection.h
  )
ENDIF ()

SET(MSSQL_HDRS
  qgsmssqlexpressioncompiler.h
)

########################################################
# Build

QT5_WRAP_CPP(MSSQL_MOC_SRCS ${MSSQL_MOC_HDRS})

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
  ${GDAL_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)

IF (WITH_GUI)
  INCLUDE_DIRECTORIES(SYSTEM
    ${QSCINTILLA_INCLUDE_DIR}
  )
ENDIF ()


ADD_LIBRARY(mssqlprovider MODULE ${MSSQL_SRCS} ${MSSQL_MOC_SRCS} ${MSSQL_HDRS})

TARGET_LINK_LIBRARIES(mssqlprovider
  qgis_core
  ${Qt5Sql_LIBRARIES}
)

IF (WITH_GUI)
  TARGET_LINK_LIBRARIES (mssqlprovider
    qgis_gui
  )
ENDIF ()

########################################################
# Install

INSTALL (TARGETS mssqlprovider
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})

