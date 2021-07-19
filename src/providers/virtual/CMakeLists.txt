
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

if (WITH_GUI)
  if (WITH_QT6)
    QT6_WRAP_UI(vlayer_provider_UI_H qgsvirtuallayersourceselectbase.ui qgsembeddedlayerselect.ui)
  else()
    QT5_WRAP_UI(vlayer_provider_UI_H qgsvirtuallayersourceselectbase.ui qgsembeddedlayerselect.ui)
  endif()
endif()

include_directories(

  ${CMAKE_BINARY_DIR}/src/ui

  ${CMAKE_CURRENT_BINARY_DIR} # ui_xxx_dlg.h
)

include_directories(SYSTEM
  ${POSTGRES_INCLUDE_DIR}
)

add_library(provider_virtuallayer MODULE
  ${QGIS_VLAYER_PROVIDER_SRCS}
  ${QGIS_VLAYER_PROVIDER_HDRS}
  ${vlayer_provider_UI_H}
  ${QGIS_VLAYER_SQL_FUNCTIONS_RCCS}
)

# require c++17
target_compile_features(provider_virtuallayer PRIVATE cxx_std_17)

target_link_libraries(provider_virtuallayer
  qgis_core
  ${${QT_VERSION_BASE}Core_LIBRARIES}
  ${${QT_VERSION_BASE}Gui_LIBRARIES}
  ${${QT_VERSION_BASE}Widgets_LIBRARIES}
  ${SQLITE3_LIBRARY}
  ${SPATIALITE_LIBRARY}
)

if (WITH_GUI)
  target_link_libraries (provider_virtuallayer
    qgis_gui
  )
  add_dependencies(provider_virtuallayer ui)
endif()

install(TARGETS provider_virtuallayer
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR}
  )
