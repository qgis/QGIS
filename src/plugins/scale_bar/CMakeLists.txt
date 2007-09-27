
########################################################
# Files

SET (SCALE_SRCS
     plugin.cpp
     plugingui.cpp
)

SET (SCALE_UIS pluginguibase.ui)

SET (SCALE_MOC_HDRS
     plugin.h
     plugingui.h
)

SET (SCALE_RCCS  scalebar_plugin.qrc)

########################################################
# Build

QT4_WRAP_UI (SCALE_UIS_H  ${SCALE_UIS})

QT4_WRAP_CPP (SCALE_MOC_SRCS  ${SCALE_MOC_HDRS})

QT4_ADD_RESOURCES(SCALE_RCC_SRCS ${SCALE_RCCS})

ADD_LIBRARY (scalebarplugin MODULE ${SCALE_SRCS} ${SCALE_MOC_SRCS} ${SCALE_RCC_SRCS} ${SCALE_UIS_H})

INCLUDE_DIRECTORIES(
     ${CMAKE_CURRENT_BINARY_DIR}
     ../../core ../../core/raster ../../core/renderer ../../core/symbology
     ../../gui
     ..
)

TARGET_LINK_LIBRARIES(scalebarplugin
  qgis_core
  qgis_gui
)


########################################################
# Install

INSTALL(TARGETS scalebarplugin
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})
