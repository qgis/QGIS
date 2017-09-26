
########################################################
# Files

SET (geometrychecker_SRCS
    qgsgeometrycheckerplugin.cpp
    qgsgeometrycheckfactory.cpp
    qgsgeometrycheckerdialog.cpp
    qgsgeometrycheckersetuptab.cpp
    qgsgeometrycheckerresulttab.cpp
    qgsgeometrycheckfixdialog.cpp
    qgsgeometrycheckerfixsummarydialog.cpp
)

SET (geometrychecker_HDRS
    qgsgeometrycheckerplugin.h
    qgsgeometrycheckfactory.h
)

SET (geometrychecker_MOC_HDRS
    qgsgeometrycheckerplugin.h
    qgsgeometrycheckerdialog.h
    qgsgeometrycheckersetuptab.h
    qgsgeometrycheckerresulttab.h
    qgsgeometrycheckfixdialog.h
    qgsgeometrycheckerfixsummarydialog.h
)

SET (geometrychecker_UIS
    qgsgeometrycheckersetuptab.ui
    qgsgeometrycheckerresulttab.ui
    qgsgeometrycheckerfixsummarydialog.ui
)

SET (geometrychecker_RCCS
    pluginres.qrc
)

########################################################
# Build

QT5_WRAP_UI (geometrychecker_UIS_H  ${geometrychecker_UIS})

QT5_WRAP_CPP (geometrychecker_MOC_SRCS  ${geometrychecker_MOC_HDRS})

QT5_ADD_RESOURCES(geometrychecker_RCC_SRCS ${geometrychecker_RCCS})

ADD_LIBRARY (geometrycheckerplugin   MODULE ${geometrychecker_HDRS} ${geometrychecker_MOC_HDRS} ${geometrychecker_SRCS} ${geometrychecker_MOC_SRCS} ${geometrychecker_RCC_SRCS} ${geometrychecker_UIS_H})

INCLUDE_DIRECTORIES(
  ${CMAKE_SOURCE_DIR}/src/core
  ${CMAKE_SOURCE_DIR}/src/core/expression
  ${CMAKE_SOURCE_DIR}/src/core/geometry
  ${CMAKE_SOURCE_DIR}/src/core/metadata
  ${CMAKE_SOURCE_DIR}/src/core/symbology
  ${CMAKE_SOURCE_DIR}/src/gui
  ${CMAKE_SOURCE_DIR}/src/analysis/vector/geometry_checker
  ${CMAKE_SOURCE_DIR}/src/plugins

  ${CMAKE_BINARY_DIR}/src/core
  ${CMAKE_BINARY_DIR}/src/gui
  ${CMAKE_BINARY_DIR}/src/analysis

  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
)
INCLUDE_DIRECTORIES(SYSTEM
  ${GEOS_INCLUDE_DIR}
  ${GDAL_INCLUDE_DIR}
)

TARGET_LINK_LIBRARIES(geometrycheckerplugin
  qgis_core
  qgis_gui
  qgis_analysis
)

########################################################
# Install

INSTALL(TARGETS geometrycheckerplugin
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})

