########################################################
# Files

SET (ZONAL_STATISTICS_SRCS
     qgszonalstatisticsplugin.cpp
     qgszonalstatisticsdialog.cpp
)

SET (ZONAL_STATISTICS_UIS
    qgszonalstatisticsdialogbase.ui
    )

SET (ZONAL_STATISTICS_MOC_HDRS
qgszonalstatisticsdialog.h
qgszonalstatisticsplugin.h
)

SET (ZONAL_STATISTICS_RCCS  zonal_statistics.qrc)

########################################################
# Build

QT4_WRAP_UI (ZONAL_STATISTICS_UIS_H  ${ZONAL_STATISTICS_UIS})

QT4_WRAP_CPP (ZONAL_STATISTICS_MOC_SRCS  ${ZONAL_STATISTICS_MOC_HDRS})

QT4_ADD_RESOURCES(ZONAL_STATISTICS_RCC_SRCS ${ZONAL_STATISTICS_RCCS})

ADD_LIBRARY (zonalstatisticsplugin MODULE
  ${ZONAL_STATISTICS_SRCS}
  ${ZONAL_STATISTICS_MOC_SRCS}
  ${ZONAL_STATISTICS_RCC_SRCS}
  ${ZONAL_STATISTICS_UIS_H})

INCLUDE_DIRECTORIES(
     ${CMAKE_CURRENT_BINARY_DIR}
     ${GDAL_INCLUDE_DIR}
     ../../core
     ../../core/geometry
     ../../core/raster
     ../../gui
     ../../analysis/vector
     ..
     .
)

TARGET_LINK_LIBRARIES(zonalstatisticsplugin
  qgis_analysis
  qgis_core
  qgis_gui
)


########################################################
# Install

INSTALL(TARGETS zonalstatisticsplugin
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR}
  )
