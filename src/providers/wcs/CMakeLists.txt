
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

add_library(wcsprovider MODULE ${WCS_SRCS})

target_link_libraries(wcsprovider
  qgis_core
)

if (WITH_GUI)
  target_link_libraries (wcsprovider
    qgis_gui
  )
endif()

install (TARGETS wcsprovider
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})
