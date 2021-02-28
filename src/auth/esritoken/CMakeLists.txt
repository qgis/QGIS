set(AUTH_ESRI_TOKEN_SRCS
  qgsauthesritokenmethod.cpp
  qgsauthesritokenedit.cpp
)

set(AUTH_ESRI_TOKEN_HDRS
  qgsauthesritokenedit.h
  qgsauthesritokenmethod.h
)

set(AUTH_ESRI_TOKEN_UIS qgsauthesritokenedit.ui)

include_directories (
  ../../core
  ../../core/auth
  ../../core/geometry
  ../../core/metadata
)
include_directories (
  ../../gui
  ../../gui/auth
  ${CMAKE_CURRENT_BINARY_DIR}
)

QT5_WRAP_UI (AUTH_ESRI_TOKEN_UIS_H ${AUTH_ESRI_TOKEN_UIS})

add_library (esritokenauthmethod MODULE ${AUTH_ESRI_TOKEN_SRCS} ${AUTH_ESRI_TOKEN_HDRS} ${AUTH_ESRI_TOKEN_UIS_H})

# require c++17
target_compile_features(esritokenauthmethod PRIVATE cxx_std_17)

target_link_libraries (esritokenauthmethod
  qgis_core
  qgis_gui
)
target_compile_definitions(esritokenauthmethod PRIVATE "-DQT_NO_FOREACH")

install(TARGETS esritokenauthmethod
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})
