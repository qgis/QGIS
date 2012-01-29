
########################################################
# Files

SET (COMPASS_SRCS
     qgscompassplugin.cpp
     qgscompassplugingui.cpp
)

SET (COMPASS_UIS qgscompasspluginguibase.ui)

SET (COMPASS_MOC_HDRS
     qgscompassplugin.h
     qgscompassplugingui.h
)

SET (COMPASS_RCCS  compass.qrc)

########################################################
# Build

QT4_WRAP_UI (COMPASS_UIS_H  ${COMPASS_UIS})

QT4_WRAP_CPP (COMPASS_MOC_SRCS  ${COMPASS_MOC_HDRS})

QT4_ADD_RESOURCES(COMPASS_RCC_SRCS ${COMPASS_RCCS})

ADD_LIBRARY (compassplugin MODULE ${COMPASS_SRCS} ${COMPASS_MOC_SRCS} ${COMPASS_RCC_SRCS} ${COMPASS_UIS_H})

INCLUDE_DIRECTORIES(
     ${CMAKE_CURRENT_BINARY_DIR}
     ../../core ../../core/raster ../../core/renderer ../../core/symbology
     ../../gui
     ..
)

TARGET_LINK_LIBRARIES(compassplugin
  qgis_core
  qgis_gui
  ${QT_MOBILITY_SENSORS_LIBRARY}
)


########################################################
# Install

INSTALL(TARGETS compassplugin
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})
