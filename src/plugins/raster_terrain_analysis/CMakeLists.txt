########################################################
# Files

SET (RASTER_TERRAIN_SRCS
     qgsrasterterrainanalysisplugin.cpp
     qgsrasterterrainanalysisdialog.cpp
)

SET (RASTER_TERRAIN_UIS
    qgsrasterterrainanalysisdialogbase.ui
    )

SET (RASTER_TERRAIN_MOC_HDRS
qgsrasterterrainanalysisdialog.h
qgsrasterterrainanalysisplugin.h
)

SET (RASTER_TERRAIN_RCCS  resources.qrc)
########################################################
# Build

QT5_WRAP_UI (RASTER_TERRAIN_UIS_H  ${RASTER_TERRAIN_UIS})

QT5_WRAP_CPP (RASTER_TERRAIN_MOC_SRCS  ${RASTER_TERRAIN_MOC_HDRS})

QT5_ADD_RESOURCES(RASTER_TERRAIN_RCC_SRCS ${RASTER_TERRAIN_RCCS})

ADD_LIBRARY (rasterterrainplugin MODULE
  ${RASTER_TERRAIN_SRCS}
  ${RASTER_TERRAIN_MOC_SRCS}
  ${RASTER_TERRAIN_RCC_SRCS}
  ${RASTER_TERRAIN_UIS_H})

INCLUDE_DIRECTORIES(
     ${CMAKE_CURRENT_BINARY_DIR}
     ../../core
     ../../core/geometry
     ../../core/raster
     ../../gui
     ../../analysis/raster
     ..
     .
)

INCLUDE_DIRECTORIES(SYSTEM
     ${GDAL_INCLUDE_DIR}
)

TARGET_LINK_LIBRARIES(rasterterrainplugin
  qgis_analysis
  qgis_core
  qgis_gui
)


########################################################
# Install

INSTALL(TARGETS rasterterrainplugin
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR}
  )
