
########################################################
# Files

SET (topol_SRCS
  topol.cpp
  rulesDialog.cpp
  checkDock.cpp
  topolError.cpp
  topolTest.cpp
  dockModel.cpp
)

SET (topol_UIS
  rulesDialog.ui
  checkDock.ui
)

SET (topol_RCCS  topol.qrc)

########################################################
# Build

QT5_WRAP_UI (topol_UIS_H  ${topol_UIS})

QT5_ADD_RESOURCES(topol_RCC_SRCS ${topol_RCCS})

ADD_LIBRARY (topolplugin MODULE ${topol_SRCS} ${topol_RCC_SRCS} ${topol_UIS_H})

INCLUDE_DIRECTORIES(SYSTEM
  ${GEOS_INCLUDE_DIR}
)

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
  ${CMAKE_CURRENT_BINARY_DIR}
)

TARGET_LINK_LIBRARIES(topolplugin
  qgis_core
  qgis_gui
)


########################################################
# Install

INSTALL(TARGETS topolplugin
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})

