########################################################
# Files

SET (GEOREF_SRCS
     qgsgeorefplugin.cpp
     qgsgeorefdatapoint.cpp
     qgsimagewarper.cpp
     qgsleastsquares.cpp
     qgspointdialog.cpp
     qgsgeorefdescriptiondialog.cpp
     qgsgeorefwarpoptionsdialog.cpp
     mapcoordsdialog.cpp
)

SET (GEOREF_UIS
     qgspointdialogbase.ui
     mapcoordsdialogbase.ui
     qgsgeorefdescriptiondialogbase.ui
     qgsgeorefwarpoptionsdialogbase.ui
)

SET (GEOREF_MOC_HDRS
     qgsgeorefplugin.h
     qgspointdialog.h
     mapcoordsdialog.h
     qgsgeorefwarpoptionsdialog.h
)

SET (GEOREF_RCCS  georeferencer.qrc)


########################################################
# Build

QT4_WRAP_UI (GEOREF_UIS_H  ${GEOREF_UIS})

QT4_WRAP_CPP (GEOREF_MOC_SRCS  ${GEOREF_MOC_HDRS})

QT4_ADD_RESOURCES(GEOREF_RCC_SRCS ${GEOREF_RCCS})

ADD_LIBRARY (georefplugin MODULE ${GEOREF_SRCS} ${GEOREF_MOC_SRCS} ${GEOREF_RCC_SRCS} ${GEOREF_UIS_H})

INCLUDE_DIRECTORIES(
     ${CMAKE_CURRENT_SOURCE_DIR}
     ${CMAKE_CURRENT_BINARY_DIR}
     ../../core ../../core/raster ../../core/renderer ../../core/symbology
     ../../gui
     ..
     ${GSL_INCLUDE_DIR}
     ${QGIS_INCLUDE_DIR}
     ${GDAL_INCLUDE_DIR}
     ${QT_QTXML_INCLUDE_DIR}
)

TARGET_LINK_LIBRARIES(georefplugin
  qgis_core
  qgis_gui
  ${GSL_LIBRARIES}
)

########################################################
# Install

INSTALL(TARGETS georefplugin
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})
