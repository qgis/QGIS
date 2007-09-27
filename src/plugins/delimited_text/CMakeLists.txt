
########################################################
# Files

SET (DLTEXT_SRCS
     qgsdelimitedtextplugin.cpp
     qgsdelimitedtextplugingui.cpp
)

SET (DLTEXT_UIS qgsdelimitedtextpluginguibase.ui)

SET (DLTEXT_MOC_HDRS
     qgsdelimitedtextplugin.h
     qgsdelimitedtextplugingui.h
)

SET (DLTEXT_RCCS  delimited_text.qrc)

########################################################
# Build

QT4_WRAP_UI (DLTEXT_UIS_H  ${DLTEXT_UIS})

QT4_WRAP_CPP (DLTEXT_MOC_SRCS  ${DLTEXT_MOC_HDRS})

QT4_ADD_RESOURCES(DLTEXT_RCC_SRCS ${DLTEXT_RCCS})

ADD_LIBRARY (delimitedtextplugin MODULE ${DLTEXT_SRCS} ${DLTEXT_MOC_SRCS} ${DLTEXT_RCC_SRCS} ${DLTEXT_UIS_H})

INCLUDE_DIRECTORIES(
     ${CMAKE_CURRENT_BINARY_DIR}
     ../../core ../../core/raster ../../core/renderer ../../core/symbology
     ../../gui
     ..
)

TARGET_LINK_LIBRARIES(delimitedtextplugin
  qgis_core
  qgis_gui
)


########################################################
# Install

INSTALL(TARGETS delimitedtextplugin
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})
