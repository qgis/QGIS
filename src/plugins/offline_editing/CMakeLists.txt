
########################################################
# Files

SET (offline_editing_plugin_SRCS
  offline_editing_plugin.cpp
  offline_editing_plugin_gui.cpp
  offline_editing_progress_dialog.cpp
)

SET (offline_editing_plugin_UIS
  offline_editing_plugin_guibase.ui
  offline_editing_progress_dialog_base.ui
)

SET (offline_editing_plugin_RCCS  offline_editing_plugin.qrc)

########################################################
# Build

QT5_WRAP_UI(offline_editing_plugin_UIS_H ${offline_editing_plugin_UIS})

QT5_ADD_RESOURCES(offline_editing_plugin_RCC_SRCS ${offline_editing_plugin_RCCS})

ADD_LIBRARY (offlineeditingplugin MODULE
  ${offline_editing_plugin_SRCS}
  ${offline_editing_plugin_RCC_SRCS}
  ${offline_editing_plugin_UIS_H}
)

TARGET_LINK_LIBRARIES(offlineeditingplugin
  qgis_core
  qgis_gui
  ${SPATIALITE_LIBRARY}
)

INCLUDE_DIRECTORIES(SYSTEM
  ${SPATIALITE_INCLUDE_DIR}
  ${SQLITE3_INCLUDE_DIR}
)

INCLUDE_DIRECTORIES(
  ${CMAKE_SOURCE_DIR}/src/core
  ${CMAKE_SOURCE_DIR}/src/core/expression
  ${CMAKE_SOURCE_DIR}/src/core/geometry
  ${CMAKE_SOURCE_DIR}/src/core/metadata
  ${CMAKE_SOURCE_DIR}/src/core/layertree
  ${CMAKE_SOURCE_DIR}/src/core/raster
  ${CMAKE_SOURCE_DIR}/src/core/symbology
  ${CMAKE_SOURCE_DIR}/src/core/effects
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

INSTALL(TARGETS offlineeditingplugin
	RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
	LIBRARY DESTINATION ${QGIS_PLUGIN_DIR}
)
