
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

SET (offline_editing_plugin_MOC_HDRS
  offline_editing_plugin.h
  offline_editing_plugin_gui.h
  offline_editing_progress_dialog.h
)

SET (offline_editing_plugin_RCCS  offline_editing_plugin.qrc)

########################################################
# Build

QT4_WRAP_UI (offline_editing_plugin_UIS_H  ${offline_editing_plugin_UIS})

QT4_WRAP_CPP (offline_editing_plugin_MOC_SRCS  ${offline_editing_plugin_MOC_HDRS})

QT4_ADD_RESOURCES(offline_editing_plugin_RCC_SRCS ${offline_editing_plugin_RCCS})

ADD_LIBRARY (offlineeditingplugin MODULE ${offline_editing_plugin_SRCS} ${offline_editing_plugin_MOC_SRCS} ${offline_editing_plugin_RCC_SRCS} ${offline_editing_plugin_UIS_H})

IF (WITH_INTERNAL_SPATIALITE)
  INCLUDE_DIRECTORIES(../../core/spatialite/headers)
  INCLUDE_DIRECTORIES(../../core/spatialite/headers/spatialite)
ELSE (WITH_INTERNAL_SPATIALITE)
  INCLUDE_DIRECTORIES(${SPATIALITE_INCLUDE_DIR})
  INCLUDE_DIRECTORIES(${SQLITE3_INCLUDE_DIR})
  TARGET_LINK_LIBRARIES(offlineeditingplugin ${SPATIALITE_LIBRARY})
  ADD_DEFINITIONS(-DEXTERNAL_SPATIALITE)
ENDIF (WITH_INTERNAL_SPATIALITE)

INCLUDE_DIRECTORIES(
  ${CMAKE_CURRENT_BINARY_DIR}
  ../../core ../../core/raster
  ../../gui
  ..
  ${GEOS_INCLUDE_DIR}
)

TARGET_LINK_LIBRARIES(offlineeditingplugin
  qgis_core
  qgis_gui
)


########################################################
# Install

INSTALL(TARGETS offlineeditingplugin
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})
