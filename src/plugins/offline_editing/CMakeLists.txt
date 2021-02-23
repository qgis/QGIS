
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

# require c++17
target_compile_features(offlineeditingplugin PRIVATE cxx_std_17)

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
  ${CMAKE_SOURCE_DIR}/src/plugins

  ${CMAKE_CURRENT_BINARY_DIR}
)

########################################################
# Install

install(TARGETS offlineeditingplugin
	RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
	LIBRARY DESTINATION ${QGIS_PLUGIN_DIR}
)
