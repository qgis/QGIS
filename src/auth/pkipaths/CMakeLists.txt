set(AUTH_PKIPATHS_SRCS
  core/qgsauthpkipathsmethod.cpp
)

set(AUTH_PKIPATHS_HDRS
  core/qgsauthpkipathsmethod.h
)

set(AUTH_PKIPATHS_UIS_H "")

if (WITH_GUI)
  set(AUTH_PKIPATHS_SRCS ${AUTH_PKIPATHS_SRCS}
    gui/qgsauthpkipathsedit.cpp
  )
  set(AUTH_PKIPATHS_HDRS ${AUTH_PKIPATHS_HDRS}
    gui/qgsauthpkipathsedit.h
  )
  set(AUTH_PKIPATHS_UIS gui/qgsauthpkipathsedit.ui)
  if (BUILD_WITH_QT6)
    QT6_WRAP_UI(AUTH_PKIPATHS_UIS_H ${AUTH_PKIPATHS_UIS})
  else()
    QT5_WRAP_UI(AUTH_PKIPATHS_UIS_H ${AUTH_PKIPATHS_UIS})
  endif()
endif()

# static library
add_library(authmethod_pkipaths_a STATIC ${AUTH_PKIPATHS_SRCS} ${AUTH_PKIPATHS_HDRS} ${AUTH_PKIPATHS_UIS_H})

target_include_directories(authmethod_pkipaths_a PUBLIC ${CMAKE_SOURCE_DIR}/src/auth/pkipaths/core)

# require c++17
target_compile_features(authmethod_pkipaths_a PRIVATE cxx_std_17)

target_link_libraries(authmethod_pkipaths_a qgis_core)

if (WITH_GUI)
  target_include_directories(authmethod_pkipaths_a PRIVATE
    ${CMAKE_SOURCE_DIR}/src/auth/pkipaths/gui
    ${CMAKE_BINARY_DIR}/src/auth/pkipaths
  )
  target_link_libraries (authmethod_pkipaths_a qgis_gui)
endif()

target_compile_definitions(authmethod_pkipaths_a PRIVATE "-DQT_NO_FOREACH")



if (FORCE_STATIC_LIBS)
  # for (external) mobile apps to be able to pick up provider for linking
  install (TARGETS authmethod_pkipaths_a ARCHIVE DESTINATION ${QGIS_PLUGIN_DIR})
else()
  # dynamically loaded module
  add_library(authmethod_pkipaths MODULE ${AUTH_PKIPATHS_SRCS} ${AUTH_PKIPATHS_HDRS} ${AUTH_PKIPATHS_UIS_H})

  # require c++17
  target_compile_features(authmethod_pkipaths PRIVATE cxx_std_17)

  target_link_libraries(authmethod_pkipaths qgis_core)

  if (WITH_GUI)
    target_include_directories(authmethod_pkipaths PRIVATE
      ${CMAKE_SOURCE_DIR}/src/auth/pkipaths/gui
      ${CMAKE_BINARY_DIR}/src/auth/pkipaths
    )
    target_link_libraries (authmethod_pkipaths qgis_gui)
    add_dependencies(authmethod_pkipaths ui)
  endif()

  target_compile_definitions(authmethod_pkipaths PRIVATE "-DQT_NO_FOREACH")

  install (TARGETS authmethod_pkipaths
    RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
    LIBRARY DESTINATION ${QGIS_PLUGIN_DIR}
  )
endif()
