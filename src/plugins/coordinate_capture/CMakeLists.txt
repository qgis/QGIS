
########################################################
# Files

SET (coordinatecapture_SRCS
     coordinatecapture.cpp
     coordinatecapturegui.cpp
     coordinatecapturemaptool.cpp
)

SET (coordinatecapture_MOC_HDRS
     coordinatecapture.h
     coordinatecapturegui.h
     coordinatecapturemaptool.h
)

SET (coordinatecapture_RCCS  coordinatecapture.qrc)

########################################################
# Build

QT4_WRAP_UI (coordinatecapture_UIS_H  ${coordinatecapture_UIS}) 

QT4_WRAP_CPP (coordinatecapture_MOC_SRCS  ${coordinatecapture_MOC_HDRS})

QT4_ADD_RESOURCES(coordinatecapture_RCC_SRCS ${coordinatecapture_RCCS})

ADD_LIBRARY (coordinatecaptureplugin MODULE ${coordinatecapture_SRCS} ${coordinatecapture_MOC_SRCS} ${coordinatecapture_RCC_SRCS})

INCLUDE_DIRECTORIES(
     ${CMAKE_CURRENT_BINARY_DIR}
     ${CMAKE_CURRENT_BINARY_DIR}/../../ui/
     ../../core 
     ../../core/raster 
     ../../core/renderer 
     ../../core/symbology
     ../../gui
     ..
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

