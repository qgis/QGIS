set(AUTH_PKIPATHS_GUI_SRCS}
    qgsauthpkipathsmethodgui.cpp
    qgsauthpkipathsedit.cpp
  )

set(AUTH_PKIPATHS_GUI_HDRS
    qgsauthpkipathsedit.h
)

set(AUTH_PKIPATHS_GUI_UIS qgsauthpkipathsedit.ui)
QT5_WRAP_UI(AUTH_PKIPATHS_GUI_UIS_H ${AUTH_PKIPATHS_GUI_UIS})
add_library(pkipathsauthmethodgui MODULE ${AUTH_PKIPATHS_GUI_SRCS} ${AUTH_PKIPATHS_GUI_HDRS} ${AUTH_PKIPATHS_GUI_UIS_H})

target_include_directories(pkipathsauthmethodgui PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
)

# require c++17
target_compile_features(pkipathsauthmethodgui PRIVATE cxx_std_17)

target_link_libraries (pkipathsauthmethodgui
    qgis_gui
)

target_compile_definitions(pkipathsauthmethodgui PRIVATE "-DQT_NO_FOREACH")

install(TARGETS pkipathsauthmethodgui
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR}
)
