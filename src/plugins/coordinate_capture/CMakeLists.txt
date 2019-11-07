
########################################################
# Files

SET (coordinatecapture_SRCS
     coordinatecapture.cpp
     coordinatecapturegui.cpp
     coordinatecapturemaptool.cpp
)
SET (coordinatecapture_RCCS  coordinatecapture.qrc)

########################################################
# Build

QT5_WRAP_UI (coordinatecapture_UIS_H  ${coordinatecapture_UIS})

QT5_ADD_RESOURCES(coordinatecapture_RCC_SRCS ${coordinatecapture_RCCS})

ADD_LIBRARY (coordinatecaptureplugin MODULE ${coordinatecapture_SRCS} ${coordinatecapture_RCC_SRCS})

INCLUDE_DIRECTORIES(
  ${CMAKE_SOURCE_DIR}/src/core
  ${CMAKE_SOURCE_DIR}/src/core/expression
  ${CMAKE_SOURCE_DIR}/src/core/geometry
  ${CMAKE_SOURCE_DIR}/src/core/layertree
  ${CMAKE_SOURCE_DIR}/src/core/metadata
  ${CMAKE_SOURCE_DIR}/src/core/raster
  ${CMAKE_SOURCE_DIR}/src/core/symbology
  ${CMAKE_SOURCE_DIR}/src/gui
  ${CMAKE_SOURCE_DIR}/src/plugins
  ${CMAKE_SOURCE_DIR}/external
  ${CMAKE_SOURCE_DIR}/external/nlohmann

  ${CMAKE_BINARY_DIR}/src/core
  ${CMAKE_BINARY_DIR}/src/gui
  ${CMAKE_BINARY_DIR}/src/ui
)
INCLUDE_DIRECTORIES(SYSTEM
)

TARGET_LINK_LIBRARIES(coordinatecaptureplugin
  qgis_core
  qgis_gui
)


########################################################
# Install

INSTALL(TARGETS coordinatecaptureplugin
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})

