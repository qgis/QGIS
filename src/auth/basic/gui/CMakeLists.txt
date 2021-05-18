set(AUTH_BASIC_GUI_SRCS
  qgsauthbasicmethodgui.cpp
  qgsauthbasicedit.cpp
)

set(AUTH_BASIC_GUI_HDRS
    qgsauthbasicedit.h
)

set(AUTH_BASIC_GUI_UIS qgsauthbasicedit.ui)
QT5_WRAP_UI(AUTH_BASIC_GUI_UIS_H ${AUTH_BASIC_GUI_UIS})

add_library(basicauthmethodgui MODULE ${AUTH_BASIC_GUI_SRCS} ${AUTH_BASIC_GUI_HDRS} ${AUTH_BASIC_GUI_UIS_H})

target_include_directories(basicauthmethodgui PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
)

# require c++17
target_compile_features(basicauthmethodgui PRIVATE cxx_std_17)

target_link_libraries (basicauthmethodgui
  qgis_core
  qgis_gui
)

target_compile_definitions(basicauthmethodgui PRIVATE "-DQT_NO_FOREACH")

install(TARGETS basicauthmethodgui
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR}
)
