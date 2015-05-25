
########################################################
# Files

SET (VRP_SRCS
  roadgraphplugin.cpp
  settingsdlg.cpp
  units.cpp
  shortestpathwidget.cpp
  linevectorlayersettings.cpp
  linevectorlayerwidget.cpp
  exportdlg.cpp
  speedproperter.cpp
)

#SET ([pluginlcasename]_UIS [pluginlcasename]guibase.ui)

SET (VRP_MOC_HDRS
  roadgraphplugin.h
  settingsdlg.h
  shortestpathwidget.h
  linevectorlayerwidget.h
  exportdlg.h
)
SET (VRP_RCCS  roadgraph.qrc)

INCLUDE_DIRECTORIES(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${GEOS_INCLUDE_DIR}
  ../../core
  ../../core/geometry
  ../../gui
  ../../analysis/network
  ..
  )

########################################################
# Build

QT4_WRAP_CPP (VRP_MOC_SRCS  ${VRP_MOC_HDRS})

QT4_ADD_RESOURCES(VRP_RCC_SRCS ${VRP_RCCS})

ADD_LIBRARY (roadgraphplugin MODULE ${VRP_SRCS} ${VRP_MOC_SRCS} ${VRP_RCC_SRCS})


TARGET_LINK_LIBRARIES(roadgraphplugin
  qgis_core
  qgis_gui
  qgis_networkanalysis
)


########################################################
# Install

INSTALL(TARGETS roadgraphplugin
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})

