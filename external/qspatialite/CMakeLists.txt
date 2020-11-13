set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${QGIS_OUTPUT_DIRECTORY}/${QGIS_PLUGIN_SUBDIR}/sqldrivers)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${QGIS_OUTPUT_DIRECTORY}/${QGIS_PLUGIN_SUBDIR}/sqldrivers)

add_definitions(${QT_DEFINITIONS})
add_definitions(-DQT_PLUGIN)
add_definitions(-DQT_NO_DEBUG)
add_definitions(-DQT_SHARED)

include_directories(SYSTEM
	${SQLITE3_INCLUDE_DIR}
	${Qt5Sql_PRIVATE_INCLUDE_DIRS}
)

set(QSQLSPATIALITE_SRC qsql_spatialite.cpp smain.cpp)
QT5_WRAP_CPP(QSQLSPATIALITE_SRC qsql_spatialite.h smain.h)

add_library(qsqlspatialite SHARED ${QSQLSPATIALITE_SRC})
target_link_libraries(qsqlspatialite
        ${Qt5Core_LIBRARIES}
        ${Qt5Sql_LIBRARIES}
	${SQLITE3_LIBRARY}
	${SPATIALITE_LIBRARY}
	qgis_core
)

install(TARGETS qsqlspatialite
  RUNTIME DESTINATION ${QT_PLUGINS_DIR}/sqldrivers
  LIBRARY DESTINATION ${QT_PLUGINS_DIR}/sqldrivers
)
