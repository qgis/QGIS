########################################################
# Files

SET (POINT_DISPLACEMENT_SRCS
     qgsdisplacementplugin.cpp
     qgspointdisplacementrenderer.cpp
     qgspointdisplacementrendererwidget.cpp
)

SET (POINT_DISPLACEMENT_UIS 
    qgspointdisplacementrendererwidgetbase.ui
    )

SET (POINT_DISPLACEMENT_MOC_HDRS
  qgspointdisplacementrendererwidget.h
)

########################################################
# Build

QT4_WRAP_UI (POINT_DISPLACEMENT_UIS_H  ${POINT_DISPLACEMENT_UIS})

QT4_WRAP_CPP (POINT_DISPLACEMENT_MOC_SRCS  ${POINT_DISPLACEMENT_MOC_HDRS})

ADD_LIBRARY (displacementplugin MODULE 
  ${POINT_DISPLACEMENT_SRCS}
${POINT_DISPLACEMENT_UIS_H}
${POINT_DISPLACEMENT_MOC_SRCS}
)

INCLUDE_DIRECTORIES(
     ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}/../../ui
     ../../core 
     ../../core/symbology-ng
     ../../core/spatialindex
    ../../gui
    ../../gui/symbology-ng
     ..
     .
)

TARGET_LINK_LIBRARIES(displacementplugin
  qgis_core
  qgis_gui
)


########################################################
# Install

INSTALL(TARGETS displacementplugin
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR}
  )
