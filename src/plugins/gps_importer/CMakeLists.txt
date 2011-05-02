
########################################################
# Files

SET (GPS_SRCS
     qgsgpsplugin.cpp
     qgsbabelformat.cpp
     qgsgpsdevice.cpp
     qgsgpsplugingui.cpp
     qgsgpsdevicedialog.cpp
)

SET (GPS_UIS
     qgsgpspluginguibase.ui
     qgsgpsdevicedialogbase.ui
)

SET (GPS_MOC_HDRS
     qgsgpsplugin.h
     qgsgpsplugingui.h
     qgsgpsdevicedialog.h
)

SET (GPS_RCCS  qgsgps_plugin.qrc)


########################################################
# Build

QT4_WRAP_UI (GPS_UIS_H  ${GPS_UIS})

QT4_WRAP_CPP (GPS_MOC_SRCS  ${GPS_MOC_HDRS})

QT4_ADD_RESOURCES(GPS_RCC_SRCS ${GPS_RCCS})

ADD_LIBRARY (gpsimporterplugin MODULE ${GPS_SRCS} ${GPS_MOC_SRCS} ${GPS_RCC_SRCS} ${GPS_UIS_H})

INCLUDE_DIRECTORIES(
     ${CMAKE_CURRENT_SOURCE_DIR}
     ${CMAKE_CURRENT_BINARY_DIR}
     ../../core ../../core/gps ../../core/gps/qextserialport

     ../../gui
     ..
     ${EXPAT_INCLUDE_DIR}
)

TARGET_LINK_LIBRARIES(gpsimporterplugin
  qgis_core
  qgis_gui
)


########################################################
# Install

INSTALL(TARGETS gpsimporterplugin
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})
