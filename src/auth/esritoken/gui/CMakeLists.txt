set(AUTH_ESRITOKEN_GUI_SRCS
  qgsauthesritokenmethodgui.cpp
  qgsauthesritokenedit.cpp
)

set(AUTH_ESRITOKEN_GUI_HDRS
  qgsauthesritokenedit.h
)


set(AUTH_ESRITOKEN_GUI_UIS qgsauthesritokenedit.ui)
QT5_WRAP_UI(AUTH_ESRITOKEN_GUI_UIS_H ${AUTH_ESRITOKEN_GUI_UIS})
add_library(esritokenauthmethodgui MODULE ${AUTH_ESRITOKEN_GUI_SRCS} ${AUTH_ESRITOKEN_GUI_HDRS} ${AUTH_ESRITOKEN_GUI_UIS_H})

# require c++17
target_compile_features(esritokenauthmethodgui PRIVATE cxx_std_17)

target_include_directories(esritokenauthmethodgui PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries (esritokenauthmethodgui
  qgis_core
  qgis_gui
)

target_compile_definitions(esritokenauthmethodgui PRIVATE "-DQT_NO_FOREACH")

install(TARGETS esritokenauthmethodgui
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR}
)
