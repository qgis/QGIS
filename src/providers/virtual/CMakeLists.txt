
########################################################
# Files

QT4_ADD_RESOURCES(QGIS_VLAYER_SQL_FUNCTIONS_RCC sqlfunctionslist.qrc)

QT4_WRAP_CPP(vlayer_provider_MOC_SRCS
  qgsvirtuallayerprovider.h
  qgsslottofunction.h
  qgsvirtuallayersourceselect.h
  qgsembeddedlayerselectdialog.h
)

QT4_WRAP_UI(vlayer_provider_UI_H qgsvirtuallayersourceselectbase.ui qgsembeddedlayerselect.ui)

SET(QGIS_VLAYER_PROVIDER_SRCS
  ${vlayer_provider_MOC_SRCS}
  ${vlayer_provider_UI_H}
  ${QGIS_VLAYER_SQL_FUNCTIONS_RCC}
  qgsvirtuallayerprovider.cpp
  qgsvirtuallayerfeatureiterator.cpp
  qgsvirtuallayerblob.cpp
  qgsvirtuallayersqlitemodule.cpp
  qgsvirtuallayersqlitehelper.cpp
  qgsvirtuallayerqueryparser.cpp
  qgsvirtuallayersourceselect.cpp
  qgsembeddedlayerselectdialog.cpp
)

INCLUDE_DIRECTORIES(
  .
  ../../core
  ../../core/auth
  ../../core/geometry
)
INCLUDE_DIRECTORIES(SYSTEM
  ${POSTGRES_INCLUDE_DIR}
  ${SQLITE3_INCLUDE_DIR}
  ${SPATIALITE_INCLUDE_DIR}
  ${GEOS_INCLUDE_DIR}
  ${QSCINTILLA_INCLUDE_DIR}
  ${QCA_INCLUDE_DIR}
)
INCLUDE_DIRECTORIES(
  ../../core
  ../../gui
  ../../gui/auth
  ../../ui
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}/../../ui
)

ADD_LIBRARY(virtuallayerprovider MODULE
  ${QGIS_VLAYER_PROVIDER_SRCS}
)

TARGET_LINK_LIBRARIES( virtuallayerprovider
  qgis_core
  qgis_gui
  ${QT_QTCORE_LIBRARY}
  ${QT_QTGUI_LIBRARY}
  ${SQLITE3_LIBRARY}
  ${SPATIALITE_LIBRARY}
)

INSTALL(TARGETS virtuallayerprovider
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR}
  )
