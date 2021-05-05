set(AUTH_BASIC_SRCS
  qgsauthbasicmethod.cpp
  qgsauthbasicedit.cpp
)

set(AUTH_BASIC_HDRS
  qgsauthbasicedit.h
  qgsauthbasicmethod.h
)

set(AUTH_BASIC_UIS qgsauthbasicedit.ui)

include_directories (
  ${CMAKE_CURRENT_BINARY_DIR}
)

QT5_WRAP_UI (AUTH_BASIC_UIS_H ${AUTH_BASIC_UIS})

add_library (basicauthmethod MODULE ${AUTH_BASIC_SRCS} ${AUTH_BASIC_HDRS} ${AUTH_BASIC_UIS_H})

# require c++17
target_compile_features(basicauthmethod PRIVATE cxx_std_17)

target_link_libraries (basicauthmethod
  qgis_core
  qgis_gui
)
target_compile_definitions(basicauthmethod PRIVATE "-DQT_NO_FOREACH")

install(TARGETS basicauthmethod
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})
