
########################################################
# Files

SET (quickprint_SRCS
     quickprintplugin.cpp
     quickprintgui.cpp
)

SET (quickprint_UIS quickprintguibase.ui)

SET (quickprint_MOC_HDRS
     quickprintplugin.h
     quickprintgui.h
)

SET (quickprint_RCCS  quickprint.qrc)

########################################################
# Build

QT4_WRAP_UI (quickprint_UIS_H  ${quickprint_UIS})

QT4_WRAP_CPP (quickprint_MOC_SRCS  ${quickprint_MOC_HDRS})

QT4_ADD_RESOURCES(quickprint_RCC_SRCS ${quickprint_RCCS})

ADD_LIBRARY (quickprintplugin MODULE 
   ${quickprint_SRCS} 
   ${quickprint_MOC_SRCS} 
   ${quickprint_RCC_SRCS} 
   ${quickprint_UIS_H})

INCLUDE_DIRECTORIES(
     ${CMAKE_CURRENT_BINARY_DIR}
     ../../core 
     ../../core/raster 
     ../../core/renderer 
     ../../core/symbology
     ../../gui
     ..
)

TARGET_LINK_LIBRARIES(quickprintplugin
  qgis_core
  qgis_gui
)


########################################################
# Install

INSTALL(TARGETS quickprintplugin
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})

