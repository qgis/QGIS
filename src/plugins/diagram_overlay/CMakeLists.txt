########################################################
# Files

SET (DIAGRAMOVERLAY_SRCS
     qgsdiagramcategory.cpp
     qgsdiagramfactory.cpp
     qgswkndiagramfactory.cpp
     qgsbardiagramfactory.cpp
     qgspiediagramfactory.cpp
     qgssvgdiagramfactory.cpp
     qgsdiagramoverlay.cpp
     qgsdiagramoverlayplugin.cpp
     qgsdiagramrenderer.cpp
)

SET (DIAGRAMOVERLAY_MOC_HDRS
     qgsdiagramoverlayplugin.h
)


########################################################
# Build

QT4_WRAP_CPP (DIAGRAMOVERLAY_MOC_SRCS  ${DIAGRAMOVERLAY_MOC_HDRS})

ADD_LIBRARY (diagramoverlay MODULE ${DIAGRAMOVERLAY_UIS_H} ${DIAGRAMOVERLAY_SRCS} ${DIAGRAMOVERLAY_MOC_SRCS})

INCLUDE_DIRECTORIES(
     ${CMAKE_CURRENT_BINARY_DIR}
     ${GEOS_INCLUDE_DIR}
     ../../core
     ../../core/pal
     ../../core/raster
     ../../core/symbology
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
