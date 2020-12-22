set(MSSQL_SRCS
  qgsmssqlconnection.cpp
  qgsmssqlprovider.cpp
  qgsmssqlproviderconnection.cpp
  qgsmssqlgeometryparser.cpp
  qgsmssqltablemodel.cpp
  qgsmssqldataitems.cpp
  qgsmssqlexpressioncompiler.cpp
  qgsmssqlfeatureiterator.cpp
  qgsmssqlgeomcolumntypethread.cpp
)

if (WITH_GUI)
  set(MSSQL_SRCS ${MSSQL_SRCS}
    qgsmssqlprovidergui.cpp
    qgsmssqldataitemguiprovider.cpp
    qgsmssqlsourceselect.cpp
    qgsmssqlnewconnection.cpp
  )
endif()

set(MSSQL_HDRS
  qgsmssqlexpressioncompiler.h
)

########################################################
# Build

include_directories(
  ${CMAKE_SOURCE_DIR}/src/core
  ${CMAKE_SOURCE_DIR}/src/core/expression
  ${CMAKE_SOURCE_DIR}/src/core/geometry
  ${CMAKE_SOURCE_DIR}/src/core/metadata
  ${CMAKE_SOURCE_DIR}/src/core/symbology
  ${CMAKE_SOURCE_DIR}/src/gui
  ${CMAKE_SOURCE_DIR}/src/gui/codeeditors
  ${CMAKE_SOURCE_DIR}/external
  ${CMAKE_SOURCE_DIR}/external/nlohmann

  ${CMAKE_BINARY_DIR}/src/core
  ${CMAKE_BINARY_DIR}/src/gui
  ${CMAKE_BINARY_DIR}/src/ui
)
include_directories(SYSTEM
  ${GDAL_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)

if (WITH_GUI)
  include_directories(SYSTEM
    ${QSCINTILLA_INCLUDE_DIR}
  )
endif()


add_library(mssqlprovider MODULE ${MSSQL_SRCS} ${MSSQL_HDRS})

target_link_libraries(mssqlprovider
  qgis_core
  ${Qt5Sql_LIBRARIES}
)

if (WITH_GUI)
  target_link_libraries (mssqlprovider
    qgis_gui
  )
  add_dependencies(mssqlprovider ui)
endif()

########################################################
# Install

install (TARGETS mssqlprovider
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})

