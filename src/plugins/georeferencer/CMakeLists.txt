FIND_PACKAGE(GSL)      # Georeferencer

IF(GSL_FOUND)

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

SET (GEOREF_MOC_HDRS
     qgsgeorefconfigdialog.h
     qgsgeorefdatapoint.h
     qgsgeorefdelegates.h
     qgsgeorefdescriptiondialog.h
     qgsgeorefplugin.h
     qgsgeorefplugingui.h
     qgsgeoreftooladdpoint.h
     qgsgeoreftooldeletepoint.h
     qgsgeoreftoolmovepoint.h
     qgsmapcoordsdialog.h
     qgstransformsettingsdialog.h
     
     qgsgcplistmodel.h
     qgsgcplistwidget.h
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
     ../../core ../../core/raster ../../core/renderer ../../core/symbology ../../core/composer
     ../../gui
     ..
     ${GSL_INCLUDE_DIR}
     ${QGIS_INCLUDE_DIR}
     ${GDAL_INCLUDE_DIR}
     ${GEOS_INCLUDE_DIR}
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

ENDIF(GSL_FOUND)
