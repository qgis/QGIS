
########################################################
# Files

SET ([pluginlcasename]_SRCS
     [pluginlcasename].cpp
     [pluginlcasename]gui.cpp
)

SET ([pluginlcasename]_UIS [pluginlcasename]guibase.ui)

SET ([pluginlcasename]_MOC_HDRS
     [pluginlcasename].h
     [pluginlcasename]gui.h
)

SET ([pluginlcasename]_RCCS  [pluginlcasename].qrc)

########################################################
# Build

QT4_WRAP_UI ([pluginlcasename]_UIS_H  ${[pluginlcasename]_UIS})

QT4_WRAP_CPP ([pluginlcasename]_MOC_SRCS  ${[pluginlcasename]_MOC_HDRS})

QT4_ADD_RESOURCES([pluginlcasename]_RCC_SRCS ${[pluginlcasename]_RCCS})

ADD_LIBRARY ([pluginlcasename]plugin MODULE ${[pluginlcasename]_SRCS} ${[pluginlcasename]_MOC_SRCS} ${[pluginlcasename]_RCC_SRCS} ${[pluginlcasename]_UIS_H})

INCLUDE_DIRECTORIES(
     ${CMAKE_CURRENT_BINARY_DIR}
     ../../core ../../core/raster ../../core/symbology
     ../../gui
     ..
)

TARGET_LINK_LIBRARIES([pluginlcasename]plugin
  qgis_core
  qgis_gui
)


########################################################
# Install

INSTALL(TARGETS [pluginlcasename]plugin
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})

