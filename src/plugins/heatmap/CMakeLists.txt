
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

QT5_WRAP_UI (heatmap_UIS_H  ${heatmap_UIS})

QT5_WRAP_CPP (heatmap_MOC_SRCS  ${heatmap_MOC_HDRS})

QT5_ADD_RESOURCES(heatmap_RCC_SRCS ${heatmap_RCCS})

ADD_LIBRARY (heatmapplugin   MODULE ${heatmap_SRCS} ${heatmap_MOC_SRCS} ${heatmap_RCC_SRCS} ${heatmap_UIS_H})

INCLUDE_DIRECTORIES( SYSTEM
     ${GEOS_INCLUDE_DIR}
     ${GDAL_INCLUDE_DIR}
)
INCLUDE_DIRECTORIES(
     ${CMAKE_CURRENT_BINARY_DIR}
     ../../core
     ../../core/geometry
     ../../core/raster
     ../../gui
     ../../analysis/raster
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

