
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

if (BUILD_WITH_QT6)
  QT6_WRAP_UI (geometrychecker_UIS_H  ${geometrychecker_UIS})
else()
  QT5_WRAP_UI (geometrychecker_UIS_H  ${geometrychecker_UIS})
endif()

add_library (plugin_geometrychecker   MODULE ${geometrychecker_SRCS}  ${geometrychecker_RCCS} ${geometrychecker_UIS_H})

# require c++17
target_compile_features(plugin_geometrychecker PRIVATE cxx_std_17)

include_directories(
  ${CMAKE_SOURCE_DIR}/src/plugins

  ${CMAKE_BINARY_DIR}/src/analysis

  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
)
include_directories(SYSTEM
  ${GDAL_INCLUDE_DIR}
)

target_link_libraries(plugin_geometrychecker
  qgis_core
  qgis_gui
  qgis_analysis
)

########################################################
# Install

install(TARGETS plugin_geometrychecker
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})

