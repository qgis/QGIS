set(qgis_agent_SRCS
  qgsagentplugin.cpp
)

add_library(plugin_qgis_agent MODULE ${qgis_agent_SRCS})

include_directories(
  ${CMAKE_SOURCE_DIR}/src/plugins
  ${CMAKE_CURRENT_BINARY_DIR}
)

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/mcp_bridge.py
  ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/qgis_agent_mcp_bridge.py
  COPYONLY
)

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/cli_adapter.py
  ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/qgis_agent_cli_adapter.py
  COPYONLY
)

target_link_libraries(plugin_qgis_agent
  qgis_core
  qgis_gui
  ${QT_VERSION_BASE}::Network
)

install(TARGETS plugin_qgis_agent
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR}
)

install(
  FILES ${CMAKE_CURRENT_SOURCE_DIR}/mcp_bridge.py
  DESTINATION ${QGIS_PLUGIN_DIR}
  RENAME qgis_agent_mcp_bridge.py
)

install(
  FILES ${CMAKE_CURRENT_SOURCE_DIR}/cli_adapter.py
  DESTINATION ${QGIS_PLUGIN_DIR}
  RENAME qgis_agent_cli_adapter.py
)
