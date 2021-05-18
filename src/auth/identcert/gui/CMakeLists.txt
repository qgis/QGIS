set(AUTH_IDENTCERT_GUI_SRCS
 qgsauthidentcertmethodgui.cpp
  qgsauthidentcertedit.cpp
)

set(AUTH_IDENTCERT_GUI_HDRS
  qgsauthidentcertedit.h
)

set(AUTH_IDENTCERT_GUI_UIS qgsauthidentcertedit.ui)
QT5_WRAP_UI(AUTH_IDENTCERT_GUI_UIS_H ${AUTH_IDENTCERT_GUI_UIS})
add_library(identcertauthmethodgui MODULE ${AUTH_IDENTCERT_GUI_SRCS} ${AUTH_IDENTCERT_GUI_HDRS} ${AUTH_IDENTCERT_GUI_UIS_H})

target_include_directories(identcertauthmethodgui PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
)

# require c++17
target_compile_features(identcertauthmethodgui PRIVATE cxx_std_17)

target_link_libraries(identcertauthmethodgui
  qgis_core
  qgis_gui
)

target_compile_definitions(identcertauthmethodgui PRIVATE "-DQT_NO_FOREACH")

install(TARGETS identcertauthmethodgui
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR}
)
