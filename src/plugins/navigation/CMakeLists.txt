
# TODO: needs 'gpsd' detection

########################################################
# Files

SET (NAVIGATION_SRCS
     navigation.cpp
     navigationgui.cpp
     gpspage.cpp
     gpscore.cpp
     positionmarker.cpp
     routingcore.cpp
)

SET (NAVIGATION_UIS
     navigationgui.ui
     gpspage.ui
)

SET (NAVIGATION_MOC_HDRS
     navigation.h
     navigationgui.h
     gpscore.h
     routingcore.h
)

SET (NAVIGATION_RCCS  navigation.qrc)

########################################################
# Build

QT4_WRAP_UI (NAVIGATION_UIS_H  ${NAVIGATION_UIS})

QT4_WRAP_CPP (NAVIGATION_MOC_SRCS  ${NAVIGATION_MOC_HDRS})

QT4_ADD_RESOURCES(NAVIGATION_RCC_SRCS ${NAVIGATION_RCCS})

ADD_LIBRARY (navigationplugin SHARED ${NAVIGATION_SRCS} ${NAVIGATION_UIS_H} ${NAVIGATION_MOC_SRCS} ${NAVIGATION_RCC_SRCS})

INCLUDE_DIRECTORIES(
     ${CMAKE_CURRENT_BINARY_DIR}
     ../../core ../../core/raster ../../core/renderer ../../core/symbology
     ../../gui
     ..
)

TARGET_LINK_LIBRARIES(navigationplugin
  ${QT_LIBRARIES}
  qgis_core
  qgis_gui
)


########################################################
# Install

INSTALL(TARGETS navigationplugin
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})
