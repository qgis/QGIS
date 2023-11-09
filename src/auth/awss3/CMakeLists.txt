set(AUTH_AWSS3_SRCS
  core/qgsauthawss3method.cpp
)

set(AUTH_AWSS3_HDRS
  core/qgsauthawss3method.h
)

set(AUTH_AWSS3_UIS_H "")

if (WITH_GUI)
  set(AUTH_AWSS3_SRCS ${AUTH_AWSS3_SRCS}
    gui/qgsauthawss3edit.cpp
  )
  set(AUTH_AWSS3_HDRS ${AUTH_AWSS3_HDRS}
    gui/qgsauthawss3edit.h
  )
  set(AUTH_AWSS3_UIS gui/qgsauthawss3edit.ui)
  if (BUILD_WITH_QT6)
    QT6_WRAP_UI(AUTH_AWSS3_UIS_H ${AUTH_AWSS3_UIS})
  else()
    QT5_WRAP_UI(AUTH_AWSS3_UIS_H ${AUTH_AWSS3_UIS})
  endif()
endif()


# static library
add_library(authmethod_awss3_a STATIC ${AUTH_AWSS3_SRCS} ${AUTH_AWSS3_HDRS} ${AUTH_AWSS3_UIS_H})

target_include_directories(authmethod_awss3_a PUBLIC ${CMAKE_SOURCE_DIR}/src/auth/awss3/core)

# require c++17
target_compile_features(authmethod_awss3_a PRIVATE cxx_std_17)

target_link_libraries(authmethod_awss3_a qgis_core)

if (WITH_GUI)
  target_include_directories(authmethod_awss3_a PRIVATE
    ${CMAKE_SOURCE_DIR}/src/auth/awss3/gui
    ${CMAKE_BINARY_DIR}/src/auth/awss3
  )

  target_link_libraries (authmethod_awss3_a qgis_gui)
endif()

target_compile_definitions(authmethod_awss3_a PRIVATE "-DQT_NO_FOREACH")



if (FORCE_STATIC_LIBS)
  # for (external) mobile apps to be able to pick up provider for linking
  install (TARGETS authmethod_awss3_a ARCHIVE DESTINATION ${QGIS_PLUGIN_DIR})
else()
  # dynamically loaded module
  add_library(authmethod_awss3 MODULE ${AUTH_AWSS3_SRCS} ${AUTH_AWSS3_HDRS} ${AUTH_AWSS3_UIS_H})

  # require c++17
  target_compile_features(authmethod_awss3 PRIVATE cxx_std_17)

  target_link_libraries(authmethod_awss3 qgis_core)

  if (WITH_GUI)
    target_include_directories(authmethod_awss3 PRIVATE
      ${CMAKE_SOURCE_DIR}/src/auth/awss3/gui
      ${CMAKE_BINARY_DIR}/src/auth/awss3
    )
    target_link_libraries (authmethod_awss3 qgis_gui)
    add_dependencies(authmethod_awss3 ui)
  endif()

  target_compile_definitions(authmethod_awss3 PRIVATE "-DQT_NO_FOREACH")

  install (TARGETS authmethod_awss3
    RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
    LIBRARY DESTINATION ${QGIS_PLUGIN_DIR}
  )
endif()
