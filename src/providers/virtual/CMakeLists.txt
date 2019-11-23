
########################################################
# Files

QT5_ADD_RESOURCES(QGIS_VLAYER_SQL_FUNCTIONS_RCC sqlfunctionslist.qrc)

SET(QGIS_VLAYER_PROVIDER_HDRS
  qgsvirtuallayerprovider.h
  qgsslottofunction.h
)

SET(QGIS_VLAYER_PROVIDER_SRCS
  qgsvirtuallayerprovider.cpp
  qgsvirtuallayerfeatureiterator.cpp
  qgsvirtuallayerblob.cpp
  qgsvirtuallayersqlitemodule.cpp
  qgsvirtuallayersqlitehelper.cpp
  qgsvirtuallayerqueryparser.cpp
)

IF (WITH_GUI)
  SET(QGIS_VLAYER_PROVIDER_SRCS ${QGIS_VLAYER_PROVIDER_SRCS}
    qgsvirtuallayersourceselect.cpp
    qgsembeddedlayerselectdialog.cpp
  )
  SET(QGIS_VLAYER_PROVIDER_HDRS ${QGIS_VLAYER_PROVIDER_HDRS}
    qgsvirtuallayersourceselect.h
    qgsembeddedlayerselectdialog.h
  )
ENDIF ()

QT5_WRAP_UI(vlayer_provider_UI_H qgsvirtuallayersourceselectbase.ui qgsembeddedlayerselect.ui)

INCLUDE_DIRECTORIES(
  ${CMAKE_SOURCE_DIR}/src/core
  ${CMAKE_SOURCE_DIR}/src/core/auth
  ${CMAKE_SOURCE_DIR}/src/core/expression
  ${CMAKE_SOURCE_DIR}/src/core/geometry
  ${CMAKE_SOURCE_DIR}/src/core/metadata
  ${CMAKE_SOURCE_DIR}/src/core/raster
  ${CMAKE_SOURCE_DIR}/src/core/symbology
  ${CMAKE_SOURCE_DIR}/src/core/effects
  ${CMAKE_SOURCE_DIR}/src/gui
  ${CMAKE_SOURCE_DIR}/src/gui/auth
  ${CMAKE_SOURCE_DIR}/external
  ${CMAKE_SOURCE_DIR}/external/nlohmann

  ${CMAKE_BINARY_DIR}/src/core
  ${CMAKE_BINARY_DIR}/src/gui
  ${CMAKE_BINARY_DIR}/src/ui

  ${CMAKE_CURRENT_BINARY_DIR} # ui_xxx_dlg.h
)

INCLUDE_DIRECTORIES(SYSTEM
  ${POSTGRES_INCLUDE_DIR}
  ${SQLITE3_INCLUDE_DIR}
  ${SPATIALITE_INCLUDE_DIR}
  ${QCA_INCLUDE_DIR}
)

IF (WITH_GUI)
  INCLUDE_DIRECTORIES(SYSTEM
    ${QSCINTILLA_INCLUDE_DIR}
  )
ENDIF ()

ADD_LIBRARY(virtuallayerprovider MODULE
  ${QGIS_VLAYER_PROVIDER_SRCS}
  ${QGIS_VLAYER_PROVIDER_HDRS}
  ${vlayer_provider_UI_H}
  ${QGIS_VLAYER_SQL_FUNCTIONS_RCC}
)

TARGET_LINK_LIBRARIES( virtuallayerprovider
  qgis_core
  ${Qt5Core_LIBRARIES}
  ${Qt5Gui_LIBRARIES}
  ${Qt5Widgets_LIBRARIES}
  ${SQLITE3_LIBRARY}
  ${SPATIALITE_LIBRARY}
)

IF (WITH_GUI)
  TARGET_LINK_LIBRARIES (virtuallayerprovider
    qgis_gui
  )
  ADD_DEPENDENCIES(virtuallayerprovider ui)
ENDIF ()

INSTALL(TARGETS virtuallayerprovider
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR}
  )
