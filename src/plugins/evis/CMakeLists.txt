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
)

SET (evis_RCCS resources/evis.qrc)

########################################################
# Build

QT4_WRAP_UI (evis_UIS_H  ${evis_UIS})

QT4_WRAP_CPP (evis_MOC_SRCS  ${evis_MOC_HDRS})

QT4_ADD_RESOURCES(evis_RCC_SRCS ${evis_RCCS})

ADD_LIBRARY (evis MODULE ${evis_SRCS} ${evis_MOC_SRCS} ${evis_RCC_SRCS} ${evis_UIS_H})

INCLUDE_DIRECTORIES(
     ${GDAL_INCLUDE_DIR}
     ${GEOS_INCLUDE_DIR}
     ${QT_INCLUDE_DIR}
     ${CMAKE_CURRENT_SOURCE_DIR}
     ${CMAKE_CURRENT_BINARY_DIR}
     ${CMAKE_CURRENT_SOURCE_DIR}/databaseconnection
     ${CMAKE_CURRENT_SOURCE_DIR}/eventbrowser
     ${CMAKE_CURRENT_SOURCE_DIR}/idtool
     ${CMAKE_CURRENT_SOURCE_DIR}/../../gui
     ${CMAKE_CURRENT_SOURCE_DIR}/../../core
     ${CMAKE_CURRENT_SOURCE_DIR}/../../core/raster
     ${CMAKE_CURRENT_SOURCE_DIR}/..
)

TARGET_LINK_LIBRARIES(evis
  qgis_core
  qgis_gui
  ${QT_QTSQL_LIBRARY}
)


########################################################
# Install

INSTALL(TARGETS evis
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})
