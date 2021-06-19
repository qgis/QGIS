set(AUTH_BASIC_SRCS
  core/qgsauthbasicmethod.cpp
)

set(AUTH_BASIC_HDRS
  core/qgsauthbasicmethod.h
)

set(AUTH_BASIC_UIS_H "")

if (WITH_GUI)
  set(AUTH_BASIC_SRCS ${AUTH_BASIC_SRCS}
    gui/qgsauthbasicmethodgui.cpp
    gui/qgsauthbasicedit.cpp
  )
  set(AUTH_BASIC_HDRS ${AUTH_BASIC_HDRS}
    gui/qgsauthbasicedit.h
  )
  set(AUTH_BASIC_UIS gui/qgsauthbasicedit.ui)
  QT5_WRAP_UI(AUTH_BASIC_UIS_H ${AUTH_BASIC_UIS})
endif()

add_library(authmethod_basic MODULE ${AUTH_BASIC_SRCS} ${AUTH_BASIC_HDRS} ${AUTH_BASIC_UIS_H})

target_include_directories(authmethod_basic PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/core
)

# require c++17
target_compile_features(authmethod_basic PRIVATE cxx_std_17)

target_link_libraries(authmethod_basic qgis_core)

if (WITH_GUI)
  target_include_directories(authmethod_basic PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/gui
    ${CMAKE_CURRENT_BINARY_DIR}
  )
  target_link_libraries(authmethod_basic qgis_gui)
endif()

target_compile_definitions(authmethod_basic PRIVATE "-DQT_NO_FOREACH")

install(TARGETS authmethod_basic
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR}
)
