SET (MSSQL_SRCS qgsmssqlprovider.cpp qgsmssqlgeometryparser.cpp qgsmssqlsourceselect.cpp qgsmssqltablemodel.cpp qgsmssqlnewconnection.cpp qgsmssqldataitems.cpp qgsmssqlfeatureiterator.cpp)
SET (MSSQL_MOC_HDRS qgsmssqlprovider.h qgsmssqlsourceselect.h qgsmssqltablemodel.h qgsmssqlnewconnection.h qgsmssqldataitems.h)

########################################################
# Build

QT4_WRAP_CPP(MSSQL_MOC_SRCS ${MSSQL_MOC_HDRS})

INCLUDE_DIRECTORIES(
  .
  ../../core
  ${GDAL_INCLUDE_DIR}
  ${GEOS_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
  ../../gui
  ${CMAKE_CURRENT_BINARY_DIR}/../../ui
)


ADD_LIBRARY(mssqlprovider MODULE ${MSSQL_SRCS} ${MSSQL_MOC_SRCS})

TARGET_LINK_LIBRARIES(mssqlprovider
  qgis_core
  qgis_gui
  ${QT_QTSQL_LIBRARY}
)
########################################################
# Install

INSTALL (TARGETS mssqlprovider
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})

