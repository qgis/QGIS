# $Id$
#
# CMake configuration file for OGR Converter plugin
# Author: Mateusz Loskot <mateusz@loskot.net>
########################################################
# Files

SET (OGR_CONVERTER_SRCS
     plugin.cpp
     dialog.cpp
     format.cpp
     translator.cpp
)

SET (OGR_CONVERTER_UIS ogrconverterguibase.ui)

SET (OGR_CONVERTER_MOC_HDRS
     plugin.h
     dialog.h
)

SET (OGR_CONVERTER_RCCS ogrconverter.qrc)

SET (OGR_CONVERTER_PLUGIN ogrconverterplugin)

########################################################
# Build

QT4_WRAP_UI (OGR_CONVERTER_UIS_H  ${OGR_CONVERTER_UIS})

QT4_WRAP_CPP (OGR_CONVERTER_MOC_SRCS  ${OGR_CONVERTER_MOC_HDRS})

QT4_ADD_RESOURCES(OGR_CONVERTER_RCC_SRCS ${OGR_CONVERTER_RCCS})

ADD_LIBRARY (${OGR_CONVERTER_PLUGIN} MODULE
    ${OGR_CONVERTER_SRCS}
    ${OGR_CONVERTER_MOC_SRCS}
    ${OGR_CONVERTER_RCC_SRCS}
    ${OGR_CONVERTER_UIS_H}
)

INCLUDE_DIRECTORIES (
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ../../core
    ../../core/raster
    ../../core/renderer
    ../../core/symbology
    ../../gui
    ..
    ${GDAL_INCLUDE_DIR}
)

TARGET_LINK_LIBRARIES(${OGR_CONVERTER_PLUGIN}
    qgis_core
    qgis_gui
)

########################################################
# Install

INSTALL(TARGETS ${OGR_CONVERTER_PLUGIN}
    RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
    LIBRARY DESTINATION ${QGIS_PLUGIN_DIR}
)
