
########################################################
# Files

SET (heatmap_SRCS
     heatmap.cpp
     heatmapgui.cpp
)

SET (heatmap_UIS heatmapguibase.ui)

SET (heatmap_MOC_HDRS
     heatmap.h
     heatmapgui.h
)

SET (heatmap_RCCS  heatmap.qrc)

########################################################
# Build

QT4_WRAP_UI (heatmap_UIS_H  ${heatmap_UIS})

QT4_WRAP_CPP (heatmap_MOC_SRCS  ${heatmap_MOC_HDRS})

QT4_ADD_RESOURCES(heatmap_RCC_SRCS ${heatmap_RCCS})

ADD_LIBRARY (heatmapplugin MODULE ${heatmap_SRCS} ${heatmap_MOC_SRCS} ${heatmap_RCC_SRCS} ${heatmap_UIS_H})

INCLUDE_DIRECTORIES(
     ${CMAKE_CURRENT_BINARY_DIR}
     ../../core ../../core/raster ../../core/renderer ../../core/symbology
     ../../gui
     ..
)

TARGET_LINK_LIBRARIES(heatmapplugin
  qgis_core
  qgis_gui
)


########################################################
# Install

INSTALL(TARGETS heatmapplugin
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})

