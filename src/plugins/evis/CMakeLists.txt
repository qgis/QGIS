########################################################
# Files

SET (evis_SRCS
     evis.cpp

     databaseconnection/evisdatabaseconnectiongui.cpp
     databaseconnection/evisdatabaseconnection.cpp
     databaseconnection/evisdatabaselayerfieldselectiongui.cpp
     databaseconnection/evisquerydefinition.cpp

     eventbrowser/evisconfiguration.cpp
     eventbrowser/evisgenericeventbrowsergui.cpp
     eventbrowser/evisimagedisplaywidget.cpp

     idtool/eviseventidtool.cpp

)

SET (evis_UIS
     ui/evisdatabaseconnectionguibase.ui
     ui/evisdatabaselayerfieldselectionguibase.ui
     ui/evisgenericeventbrowserguibase.ui
)

SET (evis_MOC_HDRS
     evis.h

     databaseconnection/evisdatabaseconnectiongui.h
     databaseconnection/evisdatabaselayerfieldselectiongui.h

     eventbrowser/evisgenericeventbrowsergui.h
     eventbrowser/evisimagedisplaywidget.h

     idtool/eviseventidtool.h
)

SET (evis_RCCS resources/evis.qrc)

########################################################
# Build

QT5_WRAP_UI (evis_UIS_H  ${evis_UIS})

QT5_WRAP_CPP (evis_MOC_SRCS  ${evis_MOC_HDRS})

QT5_ADD_RESOURCES(evis_RCC_SRCS ${evis_RCCS})

ADD_LIBRARY (evis MODULE ${evis_SRCS} ${evis_MOC_SRCS} ${evis_RCC_SRCS} ${evis_UIS_H})

INCLUDE_DIRECTORIES(SYSTEM
  ${GDAL_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)
INCLUDE_DIRECTORIES(
  ${CMAKE_CURRENT_SOURCE_DIR}/databaseconnection
  ${CMAKE_CURRENT_SOURCE_DIR}/eventbrowser
  ${CMAKE_CURRENT_SOURCE_DIR}/idtool

  ${CMAKE_SOURCE_DIR}/src/gui
  ${CMAKE_SOURCE_DIR}/src/core
  ${CMAKE_SOURCE_DIR}/src/core/expression
  ${CMAKE_SOURCE_DIR}/src/core/geometry
  ${CMAKE_SOURCE_DIR}/src/core/metadata
  ${CMAKE_SOURCE_DIR}/src/core/raster
  ${CMAKE_SOURCE_DIR}/src/plugins

  ${CMAKE_BINARY_DIR}/src/core
  ${CMAKE_BINARY_DIR}/src/gui
  ${CMAKE_CURRENT_BINARY_DIR}
)

TARGET_LINK_LIBRARIES(evis
  qgis_core
  qgis_gui
  ${Qt5Sql_LIBRARIES}
)


########################################################
# Install

INSTALL(TARGETS evis
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})
