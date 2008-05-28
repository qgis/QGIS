
########################################################
# Files

SET (WFS_SRCS
     qgswfsplugin.cpp
     qgswfssourceselect.cpp
     ../../app/qgsnewhttpconnection.cpp
)

SET (WFS_UIS qgswfssourceselectbase.ui)

SET (WFS_MOC_HDRS
     qgswfsplugin.h
     qgswfssourceselect.h
     ../../app/qgsnewhttpconnection.h
)

SET (WFS_RCCS  wfsplugin.qrc)

########################################################
# Build

QT4_WRAP_UI (WFS_UIS_H  ${WFS_UIS})

QT4_WRAP_CPP (WFS_MOC_SRCS  ${WFS_MOC_HDRS})

QT4_ADD_RESOURCES(WFS_RCC_SRCS ${WFS_RCCS})

ADD_LIBRARY (wfsplugin MODULE ${WFS_SRCS} ${WFS_MOC_SRCS} ${WFS_UIS_H} ${WFS_RCC_SRCS})

INCLUDE_DIRECTORIES(
     ${CMAKE_CURRENT_BINARY_DIR}
     ${CMAKE_CURRENT_BINARY_DIR}/../../ui
     ../../core ../../core/raster ../../core/renderer ../../core/symbology
     ../../gui
     ..
     ../../app # for QgsNewHttpConnection
     ${GEOS_INCLUDE_DIR}
)

TARGET_LINK_LIBRARIES(wfsplugin
  qgis_core
  qgis_gui
)


########################################################
# Install

INSTALL(TARGETS wfsplugin
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})
