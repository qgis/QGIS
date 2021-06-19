set(AUTH_PKCS12_SRCS
  core/qgsauthpkcs12method.cpp
)

set(AUTH_PKCS12_HDRS
  core/qgsauthpkcs12method.h
)

set(AUTH_PKCS12_UIS_H "")

if (WITH_GUI)
  set(AUTH_PKCS12_SRCS ${AUTH_PKCS12_SRCS}
    gui/qgsauthpkcs12methodgui.cpp
    gui/qgsauthpkcs12edit.cpp
  )
  set(AUTH_PKCS12_HDRS ${AUTH_PKCS12_HDRS}
    gui/qgsauthpkcs12edit.h
  )
  set(AUTH_PKCS12_UIS gui/qgsauthpkcs12edit.ui)
  QT5_WRAP_UI(AUTH_PKCS12_UIS_H ${AUTH_PKCS12_UIS})
endif()

add_library(authmethod_pkcs12 MODULE ${AUTH_PKCS12_SRCS} ${AUTH_PKCS12_HDRS} ${AUTH_PKCS12_UIS_H})

target_include_directories(authmethod_pkcs12 SYSTEM PUBLIC
  ${QCA_INCLUDE_DIR}
  ${QTKEYCHAIN_INCLUDE_DIR}
)

target_include_directories(authmethod_pkcs12 PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/core
)

# require c++17
target_compile_features(authmethod_pkcs12 PRIVATE cxx_std_17)

target_link_libraries(authmethod_pkcs12 qgis_core)

if (WITH_GUI)
  target_include_directories(authmethod_pkcs12 PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/gui
    ${CMAKE_CURRENT_BINARY_DIR}
  )
  target_link_libraries(authmethod_pkcs12 qgis_gui)
endif()

target_compile_definitions(authmethod_pkcs12 PRIVATE "-DQT_NO_FOREACH")

install(TARGETS authmethod_pkcs12
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR}
)
