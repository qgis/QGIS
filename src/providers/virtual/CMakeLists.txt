
########################################################
# Files

QT4_WRAP_CPP(vlayer_provider_MOC_SRCS qgsvirtuallayerprovider.h qgsslottofunction.h
)

QT4_WRAP_UI(vlayer_provider_UI_H qgsvirtuallayersourceselectbase.ui qgsembeddedlayerselect.ui)

SET(QGIS_VLAYER_PROVIDER_SRCS
  ${vlayer_provider_MOC_SRCS}
  qgsvirtuallayerprovider.cpp
  qgsvirtuallayerfeatureiterator.cpp
  qgsvirtuallayerblob.cpp
  qgsvirtuallayersqlitemodule.cpp
  qgsvirtuallayersqlitehelper.cpp
  qgsvirtuallayerqueryparser.cpp
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

INCLUDE_DIRECTORIES(
  .
  ../../core
  ../../core/auth
  ../../core/geometry
)
INCLUDE_DIRECTORIES(SYSTEM
  ${POSTGRES_INCLUDE_DIR}
  ${GEOS_INCLUDE_DIR}
  ${QSCINTILLA_INCLUDE_DIR}
  ${QCA_INCLUDE_DIR}
)
INCLUDE_DIRECTORIES(
  ../../core
  ../../gui
  ../../gui/auth
  ../../ui
  ${CMAKE_CURRENT_BINARY_DIR}/../../ui
)

INSTALL(TARGETS virtuallayerprovider
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR}
  )
