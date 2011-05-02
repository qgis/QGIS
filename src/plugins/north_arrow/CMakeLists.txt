
########################################################
# Files

SET (NORTH_SRCS
     plugin.cpp
     plugingui.cpp
)

SET (NORTH_UIS pluginguibase.ui)

SET (NORTH_MOC_HDRS
     plugin.h
     plugingui.h
)

SET (NORTH_RCCS  northarrow_plugin.qrc)

########################################################
# Build

QT4_WRAP_UI (NORTH_UIS_H  ${NORTH_UIS})

QT4_WRAP_CPP (NORTH_MOC_SRCS  ${NORTH_MOC_HDRS})

QT4_ADD_RESOURCES(NORTH_RCC_SRCS ${NORTH_RCCS})

ADD_LIBRARY (northarrowplugin MODULE ${NORTH_SRCS} ${NORTH_MOC_SRCS} ${NORTH_RCC_SRCS} ${NORTH_UIS_H})

INCLUDE_DIRECTORIES(
     ${CMAKE_CURRENT_BINARY_DIR}
     ../../core ../../core/raster ../../core/renderer ../../core/symbology
     ../../gui
     ..
)

TARGET_LINK_LIBRARIES(northarrowplugin
  qgis_core
  qgis_gui
)


########################################################
# Install

INSTALL(TARGETS northarrowplugin
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})
