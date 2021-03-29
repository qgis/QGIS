set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${QGIS_OUTPUT_DIRECTORY}/${QGIS_PLUGIN_SUBDIR}/sqldrivers)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${QGIS_OUTPUT_DIRECTORY}/${QGIS_PLUGIN_SUBDIR}/sqldrivers)

find_package(OCI REQUIRED)

add_definitions(${QT_DEFINITIONS})
add_definitions(-DQT_PLUGIN)
add_definitions(-DQT_SHARED)

include_directories(SYSTEM
	${OCI_INCLUDE_DIR}
	${Qt5Sql_PRIVATE_INCLUDE_DIRS}
)

set(QSQLOCISPATIAL_SRC qsql_ocispatial.cpp main.cpp qsql_ocispatial.h main.h)

add_library(qsqlocispatial SHARED ${QSQLOCISPATIAL_SRC})

target_link_libraries(qsqlocispatial
  ${Qt5Core_LIBRARIES}
  ${Qt5Sql_LIBRARIES}
  ${OCI_LIBRARY}
)

if(MSVC)
  target_link_libraries(qsqlocispatial wsock32)
endif()

install(TARGETS qsqlocispatial
  RUNTIME DESTINATION ${QT_PLUGINS_DIR}/sqldrivers
  LIBRARY DESTINATION ${QT_PLUGINS_DIR}/sqldrivers
)
