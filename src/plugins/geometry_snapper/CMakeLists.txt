
########################################################
# Files

SET (geometrysnapper_SRCS
    qgsgeometrysnapperplugin.cpp
    qgsgeometrysnapperdialog.cpp
    qgsgeometrysnapper.cpp
    qgssnapindex.cpp
)

SET (geometrysnapper_HDRS
    qgsgeometrysnapperplugin.h
    qgssnapindex.h
)

SET (geometrysnapper_UIS
    ui/qgsgeometrysnapperdialog.ui
)

SET (geometrysnapper_MOC_HDRS
    qgsgeometrysnapper.h
    qgsgeometrysnapperdialog.h
    qgsgeometrysnapperplugin.h
)

SET (geometrysnapper_RCCS
    pluginres.qrc
)

########################################################
# Build

QT4_WRAP_UI (geometrysnapper_UIS_H  ${geometrysnapper_UIS})

QT4_WRAP_CPP (geometrysnapper_MOC_SRCS  ${geometrysnapper_MOC_HDRS})

QT4_ADD_RESOURCES(geometrysnapper_RCC_SRCS ${geometrysnapper_RCCS})

ADD_LIBRARY (geometrysnapperplugin   MODULE ${geometrysnapper_HDRS} ${geometrysnapper_SRCS} ${geometrysnapper_MOC_SRCS} ${geometrysnapper_RCC_SRCS} ${geometrysnapper_UIS_H})

INCLUDE_DIRECTORIES(
     ${CMAKE_CURRENT_BINARY_DIR}
     ../../core
     ../../core/geometry
     ../../core/symbology-ng
     ../../gui
     ..
)
INCLUDE_DIRECTORIES(SYSTEM
     ${GEOS_INCLUDE_DIR}
     ${GDAL_INCLUDE_DIR}
)

TARGET_LINK_LIBRARIES(geometrysnapperplugin
  qgis_core
  qgis_gui
)

########################################################
# Install

INSTALL(TARGETS geometrysnapperplugin
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})

