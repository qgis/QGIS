
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
  ${CMAKE_SOURCE_DIR}/src/core
  ${CMAKE_SOURCE_DIR}/src/core/auth
  ${CMAKE_SOURCE_DIR}/src/core/expression
  ${CMAKE_SOURCE_DIR}/src/core/raster
  ${CMAKE_SOURCE_DIR}/src/core/geometry
  ${CMAKE_SOURCE_DIR}/src/core/metadata
  ${CMAKE_SOURCE_DIR}/src/core/symbology
  ${CMAKE_SOURCE_DIR}/src/core/layertree
  ${CMAKE_SOURCE_DIR}/src/core/textrenderer
  ${CMAKE_SOURCE_DIR}/src/gui
  ${CMAKE_SOURCE_DIR}/src/gui/auth
  ${CMAKE_SOURCE_DIR}/src/core/providers/gdal
  ${CMAKE_SOURCE_DIR}/external
  ${CMAKE_SOURCE_DIR}/external/nlohmann

  ${CMAKE_BINARY_DIR}/src/core
  ${CMAKE_BINARY_DIR}/src/gui
  ${CMAKE_BINARY_DIR}/src/ui
)
include_directories(SYSTEM
  ${GDAL_INCLUDE_DIR}
  ${QCA_INCLUDE_DIR}
  ${QTKEYCHAIN_INCLUDE_DIR}
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
