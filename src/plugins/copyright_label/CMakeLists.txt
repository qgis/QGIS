
########################################################
# Files

SET (COPYRIGHT_SRCS
     plugin.cpp
     plugingui.cpp
)

SET (COPYRIGHT_UIS pluginguibase.ui)

SET (COPYRIGHT_MOC_HDRS  plugin.h plugingui.h)

SET (COPYRIGHT_RCCS  copyright_plugin.qrc)

########################################################
# Build

QT4_WRAP_UI (COPYRIGHT_UIS_H  ${COPYRIGHT_UIS})

QT4_WRAP_CPP (COPYRIGHT_MOC_SRCS  ${COPYRIGHT_MOC_HDRS})

QT4_ADD_RESOURCES(COPYRIGHT_RCC_SRCS ${COPYRIGHT_RCCS})

ADD_LIBRARY (copyrightlabelplugin MODULE ${COPYRIGHT_SRCS} ${COPYRIGHT_MOC_SRCS} ${COPYRIGHT_RCC_SRCS} ${COPYRIGHT_UIS_H})

INCLUDE_DIRECTORIES(
     ${CMAKE_CURRENT_BINARY_DIR}
     ../../core ../../core/raster ../../core/renderer ../../core/symbology
     ../../gui
     ..
)

TARGET_LINK_LIBRARIES(copyrightlabelplugin
  qgis_core
  qgis_gui
)


########################################################
# Install

INSTALL(TARGETS copyrightlabelplugin
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})
