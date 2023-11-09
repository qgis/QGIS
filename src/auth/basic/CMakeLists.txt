set(AUTH_BASIC_SRCS
  core/qgsauthbasicmethod.cpp
)

set(AUTH_BASIC_HDRS
  core/qgsauthbasicmethod.h
)

set(AUTH_BASIC_UIS_H "")

if (WITH_GUI)
  set(AUTH_BASIC_SRCS ${AUTH_BASIC_SRCS}
    gui/qgsauthbasicedit.cpp
  )
  set(AUTH_BASIC_HDRS ${AUTH_BASIC_HDRS}
    gui/qgsauthbasicedit.h
  )
  set(AUTH_BASIC_UIS gui/qgsauthbasicedit.ui)
  if (BUILD_WITH_QT6)
    QT6_WRAP_UI(AUTH_BASIC_UIS_H ${AUTH_BASIC_UIS})
  else()
    QT5_WRAP_UI(AUTH_BASIC_UIS_H ${AUTH_BASIC_UIS})
  endif()
endif()


# static library
add_library(authmethod_basic_a STATIC ${AUTH_BASIC_SRCS} ${AUTH_BASIC_HDRS} ${AUTH_BASIC_UIS_H})

target_include_directories(authmethod_basic_a PUBLIC ${CMAKE_SOURCE_DIR}/src/auth/basic/core)

# require c++17
target_compile_features(authmethod_basic_a PRIVATE cxx_std_17)

target_link_libraries(authmethod_basic_a qgis_core)

if (WITH_GUI)
  target_include_directories(authmethod_basic_a PRIVATE
    ${CMAKE_SOURCE_DIR}/src/auth/basic/gui
    ${CMAKE_BINARY_DIR}/src/auth/basic
  )

  target_link_libraries (authmethod_basic_a qgis_gui)
endif()

target_compile_definitions(authmethod_basic_a PRIVATE "-DQT_NO_FOREACH")



if (FORCE_STATIC_LIBS)
  # for (external) mobile apps to be able to pick up provider for linking
  install (TARGETS authmethod_basic_a ARCHIVE DESTINATION ${QGIS_PLUGIN_DIR})
else()
  # dynamically loaded module
  add_library(authmethod_basic MODULE ${AUTH_BASIC_SRCS} ${AUTH_BASIC_HDRS} ${AUTH_BASIC_UIS_H})

  # require c++17
  target_compile_features(authmethod_basic PRIVATE cxx_std_17)

  target_link_libraries(authmethod_basic qgis_core)

  if (WITH_GUI)
    target_include_directories(authmethod_basic PRIVATE
      ${CMAKE_SOURCE_DIR}/src/auth/basic/gui
      ${CMAKE_BINARY_DIR}/src/auth/basic
    )
    target_link_libraries (authmethod_basic qgis_gui)
    add_dependencies(authmethod_basic ui)
  endif()

  target_compile_definitions(authmethod_basic PRIVATE "-DQT_NO_FOREACH")

  install (TARGETS authmethod_basic
    RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
    LIBRARY DESTINATION ${QGIS_PLUGIN_DIR}
  )
endif()
