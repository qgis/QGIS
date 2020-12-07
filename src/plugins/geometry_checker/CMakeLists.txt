
########################################################
# Files

set (geometrychecker_SRCS
    qgsgeometrycheckerplugin.cpp
    qgsgeometrycheckfactory.cpp
    qgsgeometrycheckerdialog.cpp
    qgsgeometrycheckersetuptab.cpp
    qgsgeometrycheckerresulttab.cpp
    qgsgeometrycheckfixdialog.cpp
    qgsgeometrycheckerfixsummarydialog.cpp
)

set (geometrychecker_HDRS
    qgsgeometrycheckerplugin.h
    qgsgeometrycheckfactory.h
)

set (geometrychecker_UIS
    qgsgeometrycheckersetuptab.ui
    qgsgeometrycheckerresulttab.ui
    qgsgeometrycheckerfixsummarydialog.ui
)

set (geometrychecker_RCCS
    pluginres.qrc
)

########################################################
# Build

QT5_WRAP_UI (geometrychecker_UIS_H  ${geometrychecker_UIS})

add_library (geometrycheckerplugin   MODULE ${geometrychecker_HDRS} ${geometrychecker_SRCS}  ${geometrychecker_RCC_SRCS} ${geometrychecker_UIS_H})

include_directories(
  ${CMAKE_SOURCE_DIR}/src/core
  ${CMAKE_SOURCE_DIR}/src/core/expression
  ${CMAKE_SOURCE_DIR}/src/core/geometry
  ${CMAKE_SOURCE_DIR}/src/core/labeling
  ${CMAKE_SOURCE_DIR}/src/core/layertree
  ${CMAKE_SOURCE_DIR}/src/core/metadata
  ${CMAKE_SOURCE_DIR}/src/core/symbology
  ${CMAKE_SOURCE_DIR}/src/core/effects
  ${CMAKE_SOURCE_DIR}/src/core/providers/ogr
  ${CMAKE_SOURCE_DIR}/src/gui
  ${CMAKE_SOURCE_DIR}/src/gui/editorwidgets
  ${CMAKE_SOURCE_DIR}/src/analysis/vector/geometry_checker
  ${CMAKE_SOURCE_DIR}/src/plugins
  ${CMAKE_SOURCE_DIR}/external
  ${CMAKE_SOURCE_DIR}/external/nlohmann

  ${CMAKE_BINARY_DIR}/src/core
  ${CMAKE_BINARY_DIR}/src/gui
  ${CMAKE_BINARY_DIR}/src/analysis

  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
)
include_directories(SYSTEM
  ${GDAL_INCLUDE_DIR}
)

target_link_libraries(geometrycheckerplugin
  qgis_core
  qgis_gui
  qgis_analysis
)

########################################################
# Install

install(TARGETS geometrycheckerplugin
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})

