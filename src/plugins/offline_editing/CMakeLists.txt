
########################################################
# Files

set (offline_editing_plugin_SRCS
  offline_editing_plugin.cpp
  offline_editing_plugin_gui.cpp
  offline_editing_progress_dialog.cpp
)

set (offline_editing_plugin_UIS
  offline_editing_plugin_guibase.ui
  offline_editing_progress_dialog_base.ui
)

set (offline_editing_plugin_RCCS  offline_editing_plugin.qrc)

########################################################
# Build

QT5_WRAP_UI(offline_editing_plugin_UIS_H ${offline_editing_plugin_UIS})

add_library (offlineeditingplugin MODULE
  ${offline_editing_plugin_SRCS}
  ${offline_editing_plugin_RCCS}
  ${offline_editing_plugin_UIS_H}
)

target_link_libraries(offlineeditingplugin
  qgis_core
  qgis_gui
  ${SPATIALITE_LIBRARY}
)

include_directories(SYSTEM
  ${SPATIALITE_INCLUDE_DIR}
  ${SQLITE3_INCLUDE_DIR}
)

include_directories(
  ${CMAKE_SOURCE_DIR}/src/core
  ${CMAKE_SOURCE_DIR}/src/core/expression
  ${CMAKE_SOURCE_DIR}/src/core/geometry
  ${CMAKE_SOURCE_DIR}/src/core/metadata
  ${CMAKE_SOURCE_DIR}/src/core/labeling
  ${CMAKE_SOURCE_DIR}/src/core/layertree
  ${CMAKE_SOURCE_DIR}/src/core/raster
  ${CMAKE_SOURCE_DIR}/src/core/symbology
  ${CMAKE_SOURCE_DIR}/src/core/effects
  ${CMAKE_SOURCE_DIR}/src/core/textrenderer
  ${CMAKE_SOURCE_DIR}/src/gui
  ${CMAKE_SOURCE_DIR}/src/gui/layertree
  ${CMAKE_SOURCE_DIR}/src/plugins
  ${CMAKE_SOURCE_DIR}/external
  ${CMAKE_SOURCE_DIR}/external/nlohmann

  ${CMAKE_BINARY_DIR}/src/core
  ${CMAKE_BINARY_DIR}/src/gui
  ${CMAKE_CURRENT_BINARY_DIR}
)

########################################################
# Install

install(TARGETS offlineeditingplugin
	RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
	LIBRARY DESTINATION ${QGIS_PLUGIN_DIR}
)
