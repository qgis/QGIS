
########################################################
# Files

set (VLAYER_SQL_FUNCTIONS_RCCS  sqlfunctionslist.qrc)

set(VLAYER_PROVIDER_SRCS
  qgsvirtuallayerprovider.cpp
  qgsvirtuallayerfeatureiterator.cpp
  qgsvirtuallayerblob.cpp
  qgsvirtuallayersqlitemodule.cpp
  qgsvirtuallayersqlitehelper.cpp
  qgsvirtuallayerqueryparser.cpp
)
set(VLAYER_PROVIDER_HDRS
  qgsvirtuallayerprovider.h
  qgsslottofunction.h
)

if (WITH_GUI)
  set(VLAYER_PROVIDER_GUI_SRCS
    qgsvirtuallayerprovidergui.cpp
    qgsvirtuallayersourceselect.cpp
    qgsembeddedlayerselectdialog.cpp
  )
  set(VLAYER_PROVIDER_GUI_HDRS
    qgsvirtuallayerprovidergui.h
    qgsvirtuallayersourceselect.h
    qgsembeddedlayerselectdialog.h
  )
endif()

########################################################
# Static
add_library(provider_virtuallayer_a STATIC ${VLAYER_PROVIDER_SRCS} ${VLAYER_PROVIDER_HDRS} ${VLAYER_SQL_FUNCTIONS_RCCS})

target_include_directories(provider_virtuallayer_a PUBLIC
  ${CMAKE_SOURCE_DIR}/src/providers/virtual
)

target_link_libraries(provider_virtuallayer_a
  qgis_core
  ${SQLITE3_LIBRARY}
  ${SPATIALITE_LIBRARY}
)

# require c++17
target_compile_features(provider_virtuallayer_a PRIVATE cxx_std_17)

target_compile_definitions(provider_virtuallayer_a PRIVATE "-DQT_NO_FOREACH")

if (WITH_GUI)
  if (BUILD_WITH_QT6)
    QT6_WRAP_UI(VLAYER_PROVIDER_UIS_H qgsvirtuallayersourceselectbase.ui qgsembeddedlayerselect.ui)
  else()
    QT5_WRAP_UI(VLAYER_PROVIDER_UIS_H qgsvirtuallayersourceselectbase.ui qgsembeddedlayerselect.ui)
  endif()
  add_library(provider_virtuallayer_gui_a STATIC ${VLAYER_PROVIDER_GUI_SRCS} ${VLAYER_PROVIDER_GUI_HDRS} ${VLAYER_PROVIDER_UIS_H})

  target_include_directories(provider_virtuallayer_gui_a PUBLIC
    ${CMAKE_BINARY_DIR}/src/providers/virtual
  )

  target_link_libraries(provider_virtuallayer_gui_a
    qgis_gui
  )

  # require c++17
  target_compile_features(provider_virtuallayer_gui_a PRIVATE cxx_std_17)

  target_compile_definitions(provider_virtuallayer_gui_a PRIVATE "-DQT_NO_FOREACH")

  add_dependencies(provider_virtuallayer_gui_a ui)

  include_directories(
    ${CMAKE_BINARY_DIR}/src/ui
    ${CMAKE_CURRENT_BINARY_DIR} # ui_xxx_dlg.h
  )
endif()


if (FORCE_STATIC_LIBS)
  # for (external) mobile apps to be able to pick up provider for linking
  install (TARGETS provider_virtuallayer_a ARCHIVE DESTINATION ${QGIS_PLUGIN_DIR})
  if (WITH_GUI)
    install (TARGETS provider_virtuallayer_gui_a ARCHIVE DESTINATION ${QGIS_PLUGIN_DIR})
  endif()
else()
  add_library(provider_virtuallayer MODULE ${VLAYER_PROVIDER_SRCS} ${VLAYER_PROVIDER_HDRS} ${VLAYER_PROVIDER_GUI_SRCS} ${VLAYER_PROVIDER_GUI_HDRS} ${VLAYER_PROVIDER_UIS_H} ${VLAYER_SQL_FUNCTIONS_RCCS})

  # require c++17
  target_compile_features(provider_virtuallayer PRIVATE cxx_std_17)

  target_compile_definitions(provider_virtuallayer PRIVATE "-DQT_NO_FOREACH")
  
  target_link_libraries(provider_virtuallayer
    qgis_core
    ${QT_VERSION_BASE}::Core
    ${QT_VERSION_BASE}::Widgets
    ${SQLITE3_LIBRARY}
    ${SPATIALITE_LIBRARY}
  )

  if (WITH_GUI)
    target_link_libraries (provider_virtuallayer
      qgis_gui
      ${QT_VERSION_BASE}::Gui
    )
    add_dependencies(provider_virtuallayer ui)
  endif()
  
  install(TARGETS provider_virtuallayer
    RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
    LIBRARY DESTINATION ${QGIS_PLUGIN_DIR}
  )
endif()
