set(AUTH_PKIPATHS_SRCS
  core/qgsauthpkipathsmethod.cpp
)

set(AUTH_PKIPATHS_HDRS
  core/qgsauthpkipathsmethod.h
)

set(AUTH_PKIPATHS_UIS_H "")

if (WITH_GUI)
  set(AUTH_PKIPATHS_SRCS ${AUTH_PKIPATHS_SRCS}
    gui/qgsauthpkipathsmethodgui.cpp
    gui/qgsauthpkipathsedit.cpp
  )
  set(AUTH_PKIPATHS_HDRS ${AUTH_PKIPATHS_HDRS}
    gui/qgsauthpkipathsedit.h
  )
  set(AUTH_PKIPATHS_UIS gui/qgsauthpkipathsedit.ui)
  QT5_WRAP_UI(AUTH_PKIPATHS_UIS_H ${AUTH_PKIPATHS_UIS})
endif()

add_library(authmethod_pkipaths MODULE ${AUTH_PKIPATHS_SRCS} ${AUTH_PKIPATHS_HDRS} ${AUTH_PKIPATHS_UIS_H})

target_include_directories (authmethod_pkipaths SYSTEM PUBLIC
  ${QCA_INCLUDE_DIR}
  ${QTKEYCHAIN_INCLUDE_DIR}
)

target_include_directories(authmethod_pkipaths PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/core
)

# require c++17
target_compile_features(authmethod_pkipaths PRIVATE cxx_std_17)

target_link_libraries(authmethod_pkipaths qgis_core)

if (WITH_GUI)
  target_include_directories(authmethod_pkipaths PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/gui
    ${CMAKE_CURRENT_BINARY_DIR}
  )
  target_link_libraries(authmethod_pkipaths qgis_gui)
endif()

target_compile_definitions(authmethod_pkipaths PRIVATE "-DQT_NO_FOREACH")

install(TARGETS authmethod_pkipaths
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR}
)
