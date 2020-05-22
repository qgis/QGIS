
########################################################
# Files

SET (COMPASS_SRCS
     compass.cpp
     qgscompassplugin.cpp
     qgscompassplugingui.cpp
)

SET (COMPASS_UIS qgscompasspluginguibase.ui)

SET (COMPASS_MOC_HDRS
     compass.h
     qgscompassplugin.h
     qgscompassplugingui.h
)

SET (COMPASS_RCCS  compass.qrc)

########################################################
# Build

QT5_WRAP_UI (COMPASS_UIS_H  ${COMPASS_UIS})

QT5_WRAP_CPP (COMPASS_MOC_SRCS  ${COMPASS_MOC_HDRS})

ADD_LIBRARY (compassplugin MODULE ${COMPASS_SRCS} ${COMPASS_MOC_SRCS} ${COMPASS_RCCS} ${COMPASS_UIS_H})

INCLUDE_DIRECTORIES(
  ${CMAKE_CURRENT_BINARY_DIR}
  ../../core ../../core/raster
  ../../gui
  ..
  ${CMAKE_BINARY_DIR}/src/core
  ${CMAKE_BINARY_DIR}/src/gui
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
