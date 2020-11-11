
########################################################
# Files

set (QGIS_VLAYER_SQL_FUNCTIONS_RCCS  sqlfunctionslist.qrc)

set(QGIS_VLAYER_PROVIDER_HDRS
  qgsvirtuallayerprovider.h
  qgsslottofunction.h
)

set(QGIS_VLAYER_PROVIDER_SRCS
  qgsvirtuallayerprovider.cpp
  qgsvirtuallayerfeatureiterator.cpp
  qgsvirtuallayerblob.cpp
  qgsvirtuallayersqlitemodule.cpp
  qgsvirtuallayersqlitehelper.cpp
  qgsvirtuallayerqueryparser.cpp
)

if (WITH_GUI)
  set(QGIS_VLAYER_PROVIDER_SRCS ${QGIS_VLAYER_PROVIDER_SRCS}
    qgsvirtuallayersourceselect.cpp
    qgsembeddedlayerselectdialog.cpp
  )
  set(QGIS_VLAYER_PROVIDER_HDRS ${QGIS_VLAYER_PROVIDER_HDRS}
    qgsvirtuallayersourceselect.h
    qgsembeddedlayerselectdialog.h
  )
endif()

QT5_WRAP_UI(vlayer_provider_UI_H qgsvirtuallayersourceselectbase.ui qgsembeddedlayerselect.ui)

include_directories(
  ${CMAKE_SOURCE_DIR}/src/core
  ${CMAKE_SOURCE_DIR}/src/core/auth
  ${CMAKE_SOURCE_DIR}/src/core/expression
  ${CMAKE_SOURCE_DIR}/src/core/geometry
  ${CMAKE_SOURCE_DIR}/src/core/metadata
  ${CMAKE_SOURCE_DIR}/src/core/raster
  ${CMAKE_SOURCE_DIR}/src/core/symbology
  ${CMAKE_SOURCE_DIR}/src/core/effects
  ${CMAKE_SOURCE_DIR}/src/core/textrenderer
  ${CMAKE_SOURCE_DIR}/src/gui
  ${CMAKE_SOURCE_DIR}/src/gui/auth
  ${CMAKE_SOURCE_DIR}/src/gui/codeeditors
  ${CMAKE_SOURCE_DIR}/external
  ${CMAKE_SOURCE_DIR}/external/nlohmann

  ${CMAKE_BINARY_DIR}/src/core
  ${CMAKE_BINARY_DIR}/src/gui
  ${CMAKE_BINARY_DIR}/src/ui

  ${CMAKE_CURRENT_BINARY_DIR} # ui_xxx_dlg.h
)

if (WITH_GUI)
  include_directories(${CMAKE_SOURCE_DIR}/src/core/labeling)
endif()

include_directories(SYSTEM
  ${POSTGRES_INCLUDE_DIR}
  ${SQLITE3_INCLUDE_DIR}
  ${SPATIALITE_INCLUDE_DIR}
  ${QCA_INCLUDE_DIR}
)

if (WITH_GUI)
  include_directories(SYSTEM
    ${QSCINTILLA_INCLUDE_DIR}
  )
endif()

add_library(virtuallayerprovider MODULE
  ${QGIS_VLAYER_PROVIDER_SRCS}
  ${QGIS_VLAYER_PROVIDER_HDRS}
  ${vlayer_provider_UI_H}
  ${QGIS_VLAYER_SQL_FUNCTIONS_RCCS}
)

target_link_libraries( virtuallayerprovider
  qgis_core
  ${Qt5Core_LIBRARIES}
  ${Qt5Gui_LIBRARIES}
  ${Qt5Widgets_LIBRARIES}
  ${SQLITE3_LIBRARY}
  ${SPATIALITE_LIBRARY}
)

if (WITH_GUI)
  target_link_libraries (virtuallayerprovider
    qgis_gui
  )
  add_dependencies(virtuallayerprovider ui)
endif()

install(TARGETS virtuallayerprovider
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR}
  )
