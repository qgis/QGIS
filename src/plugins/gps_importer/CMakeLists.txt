########################################################
# Files

set (GPS_SRCS
     qgsgpsplugin.cpp
     qgsbabelformat.cpp
     qgsgpsdevice.cpp
     qgsgpsplugingui.cpp
     qgsgpsdevicedialog.cpp
)

set (GPS_UIS
     qgsgpspluginguibase.ui
     qgsgpsdevicedialogbase.ui
)

set (GPS_RCCS  qgsgps_plugin.qrc)


########################################################
# Build

QT5_WRAP_UI (GPS_UIS_H  ${GPS_UIS})

add_library (gpsimporterplugin MODULE ${GPS_SRCS} ${GPS_RCCS} ${GPS_UIS_H})

include_directories(
  ${CMAKE_SOURCE_DIR}/src/core
  ${CMAKE_SOURCE_DIR}/src/core/expression
  ${CMAKE_SOURCE_DIR}/src/core/geometry
  ${CMAKE_SOURCE_DIR}/src/core/layertree
  ${CMAKE_SOURCE_DIR}/src/core/metadata
  ${CMAKE_SOURCE_DIR}/src/core/gps
  ${CMAKE_SOURCE_DIR}/src/core/symbology
  ${CMAKE_SOURCE_DIR}/src/gui
  ${CMAKE_SOURCE_DIR}/src/plugins
  ${CMAKE_SOURCE_DIR}/external
  ${CMAKE_SOURCE_DIR}/external/nlohmann

  ${CMAKE_BINARY_DIR}/src/core
  ${CMAKE_BINARY_DIR}/src/gui
  ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(gpsimporterplugin
  qgis_core
  qgis_gui
)

########################################################
# Install

install(TARGETS gpsimporterplugin
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})
