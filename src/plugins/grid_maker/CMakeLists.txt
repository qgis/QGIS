
########################################################
# Files

SET (GRID_COMMON_SRCS
     plugingui.cpp
     graticulecreator.cpp
)

SET (GRID_SRCS
     plugin.cpp
     ${GRID_COMMON_SRCS}
)

SET (GRID_BINARY_SRCS
     main.cpp
     ${GRID_COMMON_SRCS}
)

SET (GRID_UIS pluginguibase.ui)

SET (GRID_COMMON_MOC_HDRS plugingui.h)

SET (GRID_MOC_HDRS plugin.h)

SET (GRID_RCCS  gridmaker_plugin.qrc)

########################################################
# Build

QT4_WRAP_UI (GRID_UIS_H  ${GRID_UIS})

QT4_WRAP_CPP (GRID_COMMON_MOC_SRCS  ${GRID_COMMON_MOC_HDRS})

QT4_WRAP_CPP (GRID_MOC_SRCS  ${GRID_MOC_HDRS})

QT4_ADD_RESOURCES(GRID_RCC_SRCS ${GRID_RCCS})

ADD_LIBRARY (gridmakerplugin MODULE ${GRID_SRCS} ${GRID_MOC_SRCS} ${GRID_COMMON_MOC_SRCS} ${GRID_RCC_SRCS} ${GRID_UIS_H})

INCLUDE_DIRECTORIES(
     ${CMAKE_CURRENT_BINARY_DIR}
     ${GEOS_INCLUDE_DIR}
     ../../core 
     ../../core/raster 
     ../../core/renderer 
     ../../core/symbology
     ../../gui
     ..
)

TARGET_LINK_LIBRARIES(gridmakerplugin
  qgis_core
  qgis_gui
)

IF (PLUGINS_ALSO_BINARIES)

  ADD_EXECUTABLE (gridmaker ${GRID_BINARY_SRCS} ${GRID_COMMON_MOC_SRCS} ${GRID_RCC_SRCS})

  TARGET_LINK_LIBRARIES(gridmaker
    ${QT_LIBRARIES}
    qgis_core
    qgis_gui
  )

ENDIF (PLUGINS_ALSO_BINARIES)

########################################################
# Install

INSTALL(TARGETS gridmakerplugin
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})

IF (PLUGINS_ALSO_BINARIES)
  INSTALL(TARGETS gridmaker RUNTIME DESTINATION ${QGIS_BIN_DIR})
ENDIF (PLUGINS_ALSO_BINARIES)
