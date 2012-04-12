
########################################################
# Files

SET (dxf2shpconverter_SRCS
     dxf2shpconverter.cpp
     dxf2shpconvertergui.cpp
     builder.cpp
     getInsertions.cpp
     dxflib/src/dl_dxf.cpp
     dxflib/src/dl_writer_ascii.cpp
     )

SET (dxf2shpconverter_UIS dxf2shpconvertergui.ui)

SET (dxf2shpconverter_MOC_HDRS
     dxf2shpconverter.h
     dxf2shpconvertergui.h
     )

SET (dxf2shpconverter_RCCS  dxf2shpconverter.qrc)

########################################################
# Build

QT4_WRAP_UI (dxf2shpconverter_UIS_H  ${dxf2shpconverter_UIS})

QT4_WRAP_CPP (dxf2shpconverter_MOC_SRCS  ${dxf2shpconverter_MOC_HDRS})

QT4_ADD_RESOURCES(dxf2shpconverter_RCC_SRCS ${dxf2shpconverter_RCCS})

ADD_LIBRARY (dxf2shpconverterplugin MODULE ${dxf2shpconverter_SRCS} ${dxf2shpconverter_MOC_SRCS} ${dxf2shpconverter_RCC_SRCS} ${dxf2shpconverter_UIS_H})

INCLUDE_DIRECTORIES(
     ${CMAKE_CURRENT_BINARY_DIR}
     ../../core
     ../../core/raster
     ../../core/renderer
     ../../core/symbology
     ../../gui
     ..
)

TARGET_LINK_LIBRARIES(dxf2shpconverterplugin
  qgis_core
  qgis_gui
)


########################################################
# Install

INSTALL(TARGETS dxf2shpconverterplugin
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})

