
set (WCS_SRCS
  ../../core/providers/gdal/qgsgdalproviderbase.cpp
  qgswcsprovider.cpp
  qgswcscapabilities.cpp
  qgswcsdataitems.cpp
)

if (WITH_GUI)
  set(WCS_SRCS ${WCS_SRCS}
    qgswcsprovidergui.cpp
    qgswcsdataitemguiprovider.cpp
    qgswcssourceselect.cpp
  )
endif()

include_directories(

  ${CMAKE_BINARY_DIR}/src/ui
)

add_library(provider_wcs MODULE ${WCS_SRCS})

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
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})
