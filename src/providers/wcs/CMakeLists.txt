
set (WCS_SRCS
  ../../core/providers/gdal/qgsgdalproviderbase.cpp
  qgswcsprovider.cpp
  qgswcscapabilities.cpp
  qgswcsdataitems.cpp
)

if (WITH_GUI)
  set(WCS_GUI_SRCS
    qgswcsprovidergui.cpp
    qgswcsdataitemguiprovider.cpp
    qgswcssourceselect.cpp
  )
endif()

########################################################
# Static
add_library(provider_wcs_a STATIC ${WCS_SRCS})

target_include_directories(provider_wcs_a PUBLIC
  ${CMAKE_SOURCE_DIR}/src/providers/wcs
)

target_link_libraries(provider_wcs_a
  qgis_core
)

# require c++17
target_compile_features(provider_wcs_a PRIVATE cxx_std_17)

target_compile_definitions(provider_wcs_a PRIVATE "-DQT_NO_FOREACH")

if (WITH_GUI)
  add_library(provider_wcs_gui_a STATIC ${WCS_GUI_SRCS})

  target_include_directories(provider_wcs_gui_a PUBLIC
    ${CMAKE_BINARY_DIR}/src/providers/wcs
  )

  target_link_libraries(provider_wcs_a
    qgis_gui
  )
  target_link_libraries(provider_wcs_gui_a
    qgis_gui
  )

  # require c++17
  target_compile_features(provider_wcs_gui_a PRIVATE cxx_std_17)

  target_compile_definitions(provider_wcs_gui_a PRIVATE "-DQT_NO_FOREACH")

  add_dependencies(provider_wcs_gui_a ui)

  include_directories(SYSTEM
    ${CMAKE_BINARY_DIR}/src/ui
  )
endif()

if (FORCE_STATIC_LIBS)
  # for (external) mobile apps to be able to pick up provider for linking
  install (TARGETS provider_wcs_a ARCHIVE DESTINATION ${QGIS_PLUGIN_DIR})
  if (WITH_GUI)
    install (TARGETS provider_wcs_gui_a ARCHIVE DESTINATION ${QGIS_PLUGIN_DIR})
  endif()
else()
  add_library(provider_wcs MODULE ${WCS_SRCS} ${WCS_GUI_SRCS})
  
  # require c++17
  target_compile_features(provider_wcs PRIVATE cxx_std_17)
  
  target_link_libraries(provider_wcs
    qgis_core
  )
  
  if (WITH_GUI)
    target_link_libraries (provider_wcs
      qgis_gui
    )
  endif()
  
  install (TARGETS provider_wcs
    RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
    LIBRARY DESTINATION ${QGIS_PLUGIN_DIR}
  )
endif()
