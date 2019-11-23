FIND_PACKAGE(GSL REQUIRED)      # Georeferencer

########################################################
# Files

SET (GEOREF_SRCS
     qgsgcpcanvasitem.cpp
     qgsgeorefconfigdialog.cpp
     qgsgeorefdatapoint.cpp
     qgsgeorefdelegates.cpp
     qgsgeorefdescriptiondialog.cpp
     qgsgeorefplugin.cpp
     qgsgeorefplugingui.cpp
     qgsgeoreftooladdpoint.cpp
     qgsgeoreftooldeletepoint.cpp
     qgsgeoreftoolmovepoint.cpp
     qgsgeorefvalidators.cpp
     qgsleastsquares.cpp
     qgsmapcoordsdialog.cpp
     qgsresidualplotitem.cpp
     qgstransformsettingsdialog.cpp

     qgsgcplist.cpp
     qgsgcplistmodel.cpp
     qgsimagewarper.cpp
     qgsgeoreftransform.cpp
     qgsgcplistwidget.cpp

     qgsrasterchangecoords.cpp
)

SET (GEOREF_HDRS
     qgsvalidateddoublespinbox.h
)

SET (GEOREF_UIS
     qgsgeorefconfigdialogbase.ui
     qgsgeorefdescriptiondialogbase.ui
     qgsgeorefpluginguibase.ui
     qgsmapcoordsdialogbase.ui
     qgstransformsettingsdialogbase.ui
     ../../ui/qgsprojectionselectorbase.ui
     ../../ui/qgsrasterlayerpropertiesbase.ui
)

SET (GEOREF_RCCS  georeferencer.qrc)


########################################################
# Build

QT5_WRAP_UI (GEOREF_UIS_H  ${GEOREF_UIS})

QT5_ADD_RESOURCES(GEOREF_RCC_SRCS ${GEOREF_RCCS})

ADD_LIBRARY (georefplugin MODULE ${GEOREF_SRCS} ${GEOREF_HDRS} ${GEOREF_RCC_SRCS} ${GEOREF_UIS_H})

INCLUDE_DIRECTORIES(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/src/core
  ${CMAKE_SOURCE_DIR}/src/core/expression
  ${CMAKE_SOURCE_DIR}/src/core/geometry
  ${CMAKE_SOURCE_DIR}/src/core/layertree
  ${CMAKE_SOURCE_DIR}/src/core/metadata
  ${CMAKE_SOURCE_DIR}/src/core/raster
  ${CMAKE_SOURCE_DIR}/src/core/layout
  ${CMAKE_SOURCE_DIR}/src/core/symbology
  ${CMAKE_SOURCE_DIR}/src/core/effects
  ${CMAKE_SOURCE_DIR}/src/gui
  ${CMAKE_SOURCE_DIR}/src/gui/editorwidgets
  ${CMAKE_SOURCE_DIR}/src/gui/layertree
  ${CMAKE_SOURCE_DIR}/src/plugins
  ${CMAKE_SOURCE_DIR}/external
  ${CMAKE_SOURCE_DIR}/external/nlohmann

  ${CMAKE_BINARY_DIR}/src/core
  ${CMAKE_BINARY_DIR}/src/gui
  ${CMAKE_BINARY_DIR}/src/app
  ${CMAKE_BINARY_DIR}/src/ui

  ${CMAKE_CURRENT_BINARY_DIR}
)
INCLUDE_DIRECTORIES(SYSTEM
  ${GSL_INCLUDE_DIR}
  ${QGIS_INCLUDE_DIR}
  ${GDAL_INCLUDE_DIR}
  ${QT_QTXML_INCLUDE_DIR}
  ${QWT_INCLUDE_DIR}
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
