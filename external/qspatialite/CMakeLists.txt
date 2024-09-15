set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${QGIS_OUTPUT_DIRECTORY}/${QGIS_PLUGIN_SUBDIR}/sqldrivers)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${QGIS_OUTPUT_DIRECTORY}/${QGIS_PLUGIN_SUBDIR}/sqldrivers)

add_definitions(${QT_DEFINITIONS})
add_definitions(-DQT_PLUGIN)
add_definitions(-DQT_SHARED)

include_directories(SYSTEM ${${QT_VERSION_BASE}Sql_PRIVATE_INCLUDE_DIRS})

set(QSQLSPATIALITE_SRC qsql_spatialite.cpp smain.cpp qsql_spatialite.h smain.h)

add_library(qsqlspatialite SHARED ${QSQLSPATIALITE_SRC})

target_link_libraries(qsqlspatialite
  ${QT_VERSION_BASE}::Core
  ${QT_VERSION_BASE}::Sql
  spatialite::spatialite
  qgis_core
)

install(TARGETS qsqlspatialite
  RUNTIME DESTINATION ${QT_PLUGINS_DIR}/sqldrivers
  LIBRARY DESTINATION ${QT_PLUGINS_DIR}/sqldrivers
)
