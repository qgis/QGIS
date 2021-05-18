set(AUTH_PKCS12_GUI_SRCS}
    qgsauthpkcs12methodgui.cpp
    qgsauthpkcs12edit.cpp
  )

set(AUTH_PKCS12_GUI_HDRS
    qgsauthpkcs12edit.h
)

set(AUTH_PKCS12_GUI_UIS qgsauthpkcs12edit.ui)
QT5_WRAP_UI(AUTH_PKCS12_GUI_UIS_H ${AUTH_PKCS12_GUI_UIS})
add_library(pkcs12authmethodgui MODULE ${AUTH_PKCS12_GUI_SRCS} ${AUTH_PKCS12_GUI_HDRS} ${AUTH_PKCS12_GUI_UIS_H})

target_include_directories(pkcs12authmethodgui PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
)

# require c++17
target_compile_features(pkcs12authmethodgui PRIVATE cxx_std_17)

target_link_libraries (pkcs12authmethodgui
    qgis_gui
)

target_compile_definitions(pkcs12authmethodgui PRIVATE "-DQT_NO_FOREACH")

install(TARGETS pkcs12authmethodgui
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR}
)
