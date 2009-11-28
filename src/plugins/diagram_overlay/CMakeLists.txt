########################################################
# Files

SET (DIAGRAMOVERLAY_SRCS
     qgsdiagramdialog.cpp
     qgsdiagramcategory.cpp
     qgsdiagramfactory.cpp
     qgswkndiagramfactory.cpp
     qgswkndiagramfactorywidget.cpp
     qgsbardiagramfactory.cpp
     qgspiediagramfactory.cpp
     qgssvgdiagramfactory.cpp
     qgssvgdiagramfactorywidget.cpp
     qgsdiagramfactorywidget.cpp
     qgsdiagramoverlay.cpp
     qgsdiagramoverlayplugin.cpp
     qgsdiagramrenderer.cpp
     qgsdiagramrendererwidget.cpp
     qgslinearlyscalingdialog.cpp
)

SET (DIAGRAMOVERLAY_UIS qgsdiagramdialogbase.ui qgslinearlyscalingdialogbase.ui qgssvgdiagramfactorywidgetbase.ui qgswkndiagramfactorywidgetbase.ui)

SET (DIAGRAMOVERLAY_MOC_HDRS
     qgsdiagramoverlayplugin.h
     qgsdiagramdialog.h
     qgslinearlyscalingdialog.h
     qgssvgdiagramfactorywidget.h
     qgswkndiagramfactorywidget.h
)


########################################################
# Build

QT4_WRAP_UI (DIAGRAMOVERLAY_UIS_H  ${DIAGRAMOVERLAY_UIS})

QT4_WRAP_CPP (DIAGRAMOVERLAY_MOC_SRCS  ${DIAGRAMOVERLAY_MOC_HDRS})

ADD_LIBRARY (diagramoverlay MODULE ${DIAGRAMOVERLAY_UIS_H} ${DIAGRAMOVERLAY_SRCS} ${DIAGRAMOVERLAY_MOC_SRCS})

INCLUDE_DIRECTORIES(
     ${CMAKE_CURRENT_BINARY_DIR} ${GEOS_INCLUDE_DIR}
     ../../core ../../core/pal ../../core/raster ../../core/renderer ../../core/symbology
     ../../gui
     ..
)

TARGET_LINK_LIBRARIES(diagramoverlay
  qgis_core
  qgis_gui
)


########################################################
# Install

INSTALL(TARGETS diagramoverlay
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})
