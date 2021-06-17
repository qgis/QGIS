set(AUTH_IDENTCERT_SRCS
  core/qgsauthidentcertmethod.cpp
)

set(AUTH_IDENTCERT_HDRS
  core/qgsauthidentcertmethod.h
)

set(AUTH_IDENTCERT_UIS_H "")

if (WITH_GUI)
  set(AUTH_IDENTCERT_SRCS ${AUTH_IDENTCERT_SRCS}
    gui/qgsauthidentcertmethodgui.cpp
    gui/qgsauthidentcertedit.cpp
  )
  set(AUTH_IDENTCERT_HDRS ${AUTH_IDENTCERT_HDRS}
    gui/qgsauthidentcertedit.h
  )
  set(AUTH_IDENTCERT_UIS gui/qgsauthidentcertedit.ui)
  QT5_WRAP_UI(AUTH_IDENTCERT_UIS_H ${AUTH_IDENTCERT_UIS})
endif()

add_library(authmethod_identcert MODULE ${AUTH_IDENTCERT_SRCS} ${AUTH_IDENTCERT_HDRS} ${AUTH_IDENTCERT_UIS_H})

target_include_directories(authmethod_identcert SYSTEM PUBLIC
  ${QCA_INCLUDE_DIR}
  ${QTKEYCHAIN_INCLUDE_DIR}
)

target_include_directories(authmethod_identcert PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/core
)

# require c++17
target_compile_features(authmethod_identcert PRIVATE cxx_std_17)

target_link_libraries(authmethod_identcert qgis_core)

if (WITH_GUI)
  target_include_directories(authmethod_identcert PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/gui
    ${CMAKE_CURRENT_BINARY_DIR}
  )
  target_link_libraries(authmethod_identcert qgis_gui)
endif()

target_compile_definitions(authmethod_identcert PRIVATE "-DQT_NO_FOREACH")

install(TARGETS authmethod_identcert
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR}
)
