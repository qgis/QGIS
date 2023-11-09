set(AUTH_ESRITOKEN_SRCS
  core/qgsauthesritokenmethod.cpp
)

set(AUTH_ESRITOKEN_HDRS
  core/qgsauthesritokenmethod.h
)

set(AUTH_ESRITOKEN_UIS_H "")

if (WITH_GUI)
  set(AUTH_ESRITOKEN_SRCS ${AUTH_ESRITOKEN_SRCS}
    gui/qgsauthesritokenedit.cpp
  )
  set(AUTH_ESRITOKEN_HDRS ${AUTH_ESRITOKEN_HDRS}
    gui/qgsauthesritokenedit.h
  )
  set(AUTH_ESRITOKEN_UIS gui/qgsauthesritokenedit.ui)
  if (BUILD_WITH_QT6)
    QT6_WRAP_UI(AUTH_ESRITOKEN_UIS_H ${AUTH_ESRITOKEN_UIS})
  else()
    QT5_WRAP_UI(AUTH_ESRITOKEN_UIS_H ${AUTH_ESRITOKEN_UIS})
  endif()
endif()


# static library
add_library(authmethod_esritoken_a STATIC ${AUTH_ESRITOKEN_SRCS} ${AUTH_ESRITOKEN_HDRS} ${AUTH_ESRITOKEN_UIS_H})

target_include_directories(authmethod_esritoken_a PUBLIC ${CMAKE_SOURCE_DIR}/src/auth/esritoken/core)

# require c++17
target_compile_features(authmethod_esritoken_a PRIVATE cxx_std_17)

target_link_libraries(authmethod_esritoken_a qgis_core)

if (WITH_GUI)
  target_include_directories(authmethod_esritoken_a PRIVATE
    ${CMAKE_SOURCE_DIR}/src/auth/esritoken/gui
    ${CMAKE_BINARY_DIR}/src/auth/esritoken
  )
  target_link_libraries (authmethod_esritoken_a qgis_gui)
endif()

target_compile_definitions(authmethod_esritoken_a PRIVATE "-DQT_NO_FOREACH")

if (FORCE_STATIC_LIBS)
  # for (external) mobile apps to be able to pick up provider for linking
  install (TARGETS authmethod_esritoken_a ARCHIVE DESTINATION ${QGIS_PLUGIN_DIR})
else()
  # dynamically loaded module
  add_library(authmethod_esritoken MODULE ${AUTH_ESRITOKEN_SRCS} ${AUTH_ESRITOKEN_HDRS} ${AUTH_ESRITOKEN_UIS_H})

  # require c++17
  target_compile_features(authmethod_esritoken PRIVATE cxx_std_17)

  target_link_libraries(authmethod_esritoken qgis_core)

  if (WITH_GUI)
    target_include_directories(authmethod_esritoken PRIVATE
      ${CMAKE_SOURCE_DIR}/src/auth/esritoken/gui
      ${CMAKE_BINARY_DIR}/src/auth/esritoken
    )
    target_link_libraries (authmethod_esritoken qgis_gui)
    add_dependencies(authmethod_esritoken ui)
  endif()

  target_compile_definitions(authmethod_esritoken PRIVATE "-DQT_NO_FOREACH")

  install (TARGETS authmethod_esritoken
    RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
    LIBRARY DESTINATION ${QGIS_PLUGIN_DIR}
  )
endif()
