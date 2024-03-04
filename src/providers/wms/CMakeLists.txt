set (WMS_SRCS
  qgswmscapabilities.cpp
  qgswmsprovider.cpp
  qgswmsconnection.cpp
  qgswmsdataitems.cpp
  qgsxyzconnection.cpp
)

if (WITH_GUI)
  set(WMS_GUI_SRCS
    qgswmsprovidergui.cpp
    qgswmssourceselect.cpp
    qgswmsdataitemguiproviders.cpp
    qgswmstsettingswidget.cpp
    qgstilescalewidget.cpp
    qgswmtsdimensions.cpp
    qgsxyzconnectiondialog.cpp
    qgsxyzsourceselect.cpp
    qgsxyzsourcewidget.cpp
  )

  set(WMS_UIS ${CMAKE_SOURCE_DIR}/src/ui/qgswmstsettingswidgetbase.ui)
endif()

# static library
add_library(provider_wms_a STATIC ${WMS_SRCS})

target_include_directories(provider_wms_a PUBLIC
  ${CMAKE_SOURCE_DIR}/src/providers/wms
)

# require c++17
target_compile_features(provider_wms_a PRIVATE cxx_std_17)

if (WITH_GUI)
  if (BUILD_WITH_QT6)
    QT6_WRAP_UI(WMS_UIS_H ${WMS_UIS})
  else()
    QT5_WRAP_UI(WMS_UIS_H ${WMS_UIS})
  endif()

  add_library(provider_wms_gui_a STATIC ${WMS_GUI_SRCS} ${WMS_UIS_H})

  target_include_directories (provider_wms_gui_a PUBLIC
    ${CMAKE_BINARY_DIR}/src/providers/wms
  )

  # require c++17
  target_compile_features(provider_wms_gui_a PRIVATE cxx_std_17)

  add_dependencies(provider_wms_a ui)
  add_dependencies(provider_wms_gui_a ui)
endif()

target_link_libraries(provider_wms_a
  qgis_core
)

# We use private headers from core that need this
target_compile_definitions(provider_wms_a PRIVATE "CMAKE_SOURCE_DIR=\"${CMAKE_SOURCE_DIR}\"")
target_compile_definitions(provider_wms_a PRIVATE "-DQT_NO_FOREACH")

if (WITH_GUI)
  target_link_libraries(provider_wms_gui_a
    qgis_gui
    provider_wms_a
  )
  target_compile_definitions(provider_wms_gui_a PRIVATE "-DQT_NO_FOREACH")
endif()

if (FORCE_STATIC_LIBS)
  # for (external) mobile apps to be able to pick up provider for linking
  install (TARGETS provider_wms_a ARCHIVE DESTINATION ${QGIS_PLUGIN_DIR})
  if (WITH_GUI)
    install (TARGETS provider_wms_gui_a ARCHIVE DESTINATION ${QGIS_PLUGIN_DIR})
  endif()
else()
  # dynamically loaded module
  add_library(provider_wms MODULE ${WMS_SRCS} ${WMS_GUI_SRCS})

  # require c++17
  target_compile_features(provider_wms PRIVATE cxx_std_17)

  target_link_libraries(provider_wms
    qgis_core
  )

  if (WITH_GUI)
    target_link_libraries (provider_wms
      qgis_gui
    )
    add_dependencies(provider_wms ui)
  endif()

  target_compile_definitions(provider_wms PRIVATE "-DQT_NO_FOREACH")
  # We use private headers from core that need this
  target_compile_definitions(provider_wms PRIVATE "CMAKE_SOURCE_DIR=\"${CMAKE_SOURCE_DIR}\"")

  install (TARGETS provider_wms
    RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
    LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})

endif()
