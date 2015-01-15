
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

QT4_WRAP_UI(offline_editing_plugin_UIS_H ${offline_editing_plugin_UIS})

QT4_WRAP_CPP(offline_editing_plugin_MOC_SRCS ${offline_editing_plugin_MOC_HDRS})

QT4_ADD_RESOURCES(offline_editing_plugin_RCC_SRCS ${offline_editing_plugin_RCCS})

ADD_LIBRARY (offlineeditingplugin MODULE
	${offline_editing_plugin_SRCS}
	${offline_editing_plugin_MOC_SRCS}
	${offline_editing_plugin_RCC_SRCS}
	${offline_editing_plugin_UIS_H}
)

TARGET_LINK_LIBRARIES(offlineeditingplugin
	qgis_core
	qgis_gui
	${SPATIALITE_LIBRARY}
)

INCLUDE_DIRECTORIES(
	${SPATIALITE_INCLUDE_DIR}
	${SQLITE3_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	../../core
	../../core/layertree
	../../core/raster
	../../gui
	../../gui/layertree
	..
	${GEOS_INCLUDE_DIR}
)

########################################################
# Install

INSTALL(TARGETS offlineeditingplugin
	RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
	LIBRARY DESTINATION ${QGIS_PLUGIN_DIR}
)
