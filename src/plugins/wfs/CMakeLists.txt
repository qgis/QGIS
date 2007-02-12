
########################################################
# Files

SET (WFS_SRCS
     qgswfsplugin.cpp
     qgswfssourceselect.cpp
     ../../gui/qgsnewhttpconnection.cpp
)

SET (WFS_UIS qgswfssourceselectbase.ui)

SET (WFS_MOC_HDRS
     qgswfsplugin.h
     qgswfssourceselect.h
     ../../gui/qgsnewhttpconnection.h
)

#SET (WFS_RCCS  northarrow_plugin.qrc)

########################################################
# Build

QT4_WRAP_UI (WFS_UIS_H  ${WFS_UIS})

QT4_WRAP_CPP (WFS_MOC_SRCS  ${WFS_MOC_HDRS})

#QT4_ADD_RESOURCES(WFS_RCC_SRCS ${WFS_RCCS})

ADD_LIBRARY (wfsplugin MODULE ${WFS_SRCS} ${WFS_MOC_SRCS} ${WFS_UIS_H})

INCLUDE_DIRECTORIES(
     ${CMAKE_CURRENT_BINARY_DIR}
     ${CMAKE_CURRENT_BINARY_DIR}/../../ui
     ../../core ../../core/raster ../../core/renderer ../../core/symbology
     ../../gui
     ../../widgets/projectionselector
     ../../providers/wfs
     ..
     ${GEOS_INCLUDE_DIR}
     ${GEOS_INCLUDE_DIR}/geos
     ${PROJ_INCLUDE_DIR}
     ${GDAL_INCLUDE_DIR}
)

TARGET_LINK_LIBRARIES(wfsplugin
  ${QT_LIBRARIES}
  ${GEOS_LIBRARY}
  ${PROJ_LIBRARY}
  ${GDAL_LIBRARY}
  qgis_core
)


########################################################
# Install

INSTALL(TARGETS wfsplugin
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})
