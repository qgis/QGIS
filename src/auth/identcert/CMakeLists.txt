set(AUTH_IDENTCERT_SRCS
  core/qgsauthidentcertmethod.cpp
)

set(AUTH_IDENTCERT_HDRS
  core/qgsauthidentcertmethod.h
)

set(AUTH_IDENTCERT_UIS_H "")

if (WITH_GUI)
  set(AUTH_IDENTCERT_SRCS ${AUTH_IDENTCERT_SRCS}
    gui/qgsauthidentcertedit.cpp
  )
  set(AUTH_IDENTCERT_HDRS ${AUTH_IDENTCERT_HDRS}
    gui/qgsauthidentcertedit.h
  )
  set(AUTH_IDENTCERT_UIS gui/qgsauthidentcertedit.ui)
  if (BUILD_WITH_QT6)
    QT6_WRAP_UI(AUTH_IDENTCERT_UIS_H ${AUTH_IDENTCERT_UIS})
  else()
    QT5_WRAP_UI(AUTH_IDENTCERT_UIS_H ${AUTH_IDENTCERT_UIS})
  endif()
endif()


# static library
add_library(authmethod_identcert_a STATIC ${AUTH_IDENTCERT_SRCS} ${AUTH_IDENTCERT_HDRS} ${AUTH_IDENTCERT_UIS_H})

target_include_directories(authmethod_identcert_a PUBLIC ${CMAKE_SOURCE_DIR}/src/auth/identcert/core)

# require c++17
target_compile_features(authmethod_identcert_a PRIVATE cxx_std_17)

target_link_libraries(authmethod_identcert_a qgis_core)

if (WITH_GUI)
  target_include_directories(authmethod_identcert_a PRIVATE
    ${CMAKE_SOURCE_DIR}/src/auth/identcert/gui
    ${CMAKE_BINARY_DIR}/src/auth/identcert
  )
  target_link_libraries (authmethod_identcert_a qgis_gui)
endif()

target_compile_definitions(authmethod_identcert_a PRIVATE "-DQT_NO_FOREACH")



if (FORCE_STATIC_LIBS)
  # for (external) mobile apps to be able to pick up provider for linking
  install (TARGETS authmethod_identcert_a ARCHIVE DESTINATION ${QGIS_PLUGIN_DIR})
else()
  # dynamically loaded module
  add_library(authmethod_identcert MODULE ${AUTH_IDENTCERT_SRCS} ${AUTH_IDENTCERT_HDRS} ${AUTH_IDENTCERT_UIS_H})

  # require c++17
  target_compile_features(authmethod_identcert PRIVATE cxx_std_17)

  target_link_libraries(authmethod_identcert qgis_core)

  if (WITH_GUI)
    target_include_directories(authmethod_identcert PRIVATE
      ${CMAKE_SOURCE_DIR}/src/auth/identcert/gui
      ${CMAKE_BINARY_DIR}/src/auth/identcert
    )
    target_link_libraries (authmethod_identcert qgis_gui)
    add_dependencies(authmethod_identcert ui)
  endif()

  target_compile_definitions(authmethod_identcert PRIVATE "-DQT_NO_FOREACH")

  install (TARGETS authmethod_identcert
    RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
    LIBRARY DESTINATION ${QGIS_PLUGIN_DIR}
  )
endif()
