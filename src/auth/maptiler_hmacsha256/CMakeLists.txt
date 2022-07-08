set(AUTH_MAPTILER_HMACSHA256_SRCS
  core/qgsauthmaptilerhmacsha256method.cpp
)

set(AUTH_MAPTILER_HMACSHA256_HDRS
  core/qgsauthmaptilerhmacsha256method.h
)

set(AUTH_MAPTILER_HMACSHA256_UIS_H "")

if (WITH_GUI)
  set(AUTH_MAPTILER_HMACSHA256_SRCS ${AUTH_MAPTILER_HMACSHA256_SRCS}
    gui/qgsauthmaptilerhmacsha256edit.cpp
  )
  set(AUTH_MAPTILER_HMACSHA256_HDRS ${AUTH_MAPTILER_HMACSHA256_HDRS}
    gui/qgsauthmaptilerhmacsha256edit.h
  )
  set(AUTH_MAPTILER_HMACSHA256_UIS gui/qgsauthmaptilerhmacsha256edit.ui)
  if (BUILD_WITH_QT6)
    QT6_WRAP_UI(AUTH_MAPTILER_HMACSHA256_UIS_H ${AUTH_MAPTILER_HMACSHA256_UIS})
  else()
    QT5_WRAP_UI(AUTH_MAPTILER_HMACSHA256_UIS_H ${AUTH_MAPTILER_HMACSHA256_UIS})
  endif()
endif()


# static library
add_library(authmethod_maptilerhmacsha256_a STATIC ${AUTH_MAPTILER_HMACSHA256_SRCS} ${AUTH_MAPTILER_HMACSHA256_HDRS} ${AUTH_MAPTILER_HMACSHA256_UIS_H})

target_include_directories(authmethod_maptilerhmacsha256_a PUBLIC ${CMAKE_SOURCE_DIR}/src/auth/hmacsha256/core)

# require c++17
target_compile_features(authmethod_maptilerhmacsha256_a PRIVATE cxx_std_17)

target_link_libraries(authmethod_maptilerhmacsha256_a qgis_core)

if (WITH_GUI)
  target_include_directories(authmethod_maptilerhmacsha256_a PRIVATE
    ${CMAKE_SOURCE_DIR}/src/auth/maptiler_hmacsha256/gui
    ${CMAKE_BINARY_DIR}/src/auth/maptiler_hmacsha256
  )
  target_link_libraries (authmethod_maptilerhmacsha256_a qgis_gui)
endif()

target_compile_definitions(authmethod_maptilerhmacsha256_a PRIVATE "-DQT_NO_FOREACH")

if (FORCE_STATIC_LIBS)
  # for (external) mobile apps to be able to pick up provider for linking
  install (TARGETS authmethod_maptilerhmacsha256_a ARCHIVE DESTINATION ${QGIS_PLUGIN_DIR})
else()
  # dynamically loaded module
  add_library(authmethod_maptilerhmacsha256 MODULE ${AUTH_MAPTILER_HMACSHA256_SRCS} ${AUTH_MAPTILER_HMACSHA256_HDRS} ${AUTH_MAPTILER_HMACSHA256_UIS_H})

  # require c++17
  target_compile_features(authmethod_maptilerhmacsha256 PRIVATE cxx_std_17)

  target_link_libraries(authmethod_maptilerhmacsha256 qgis_core)

  if (WITH_GUI)
    target_include_directories(authmethod_maptilerhmacsha256 PRIVATE
      ${CMAKE_SOURCE_DIR}/src/auth/maptiler_hmacsha256/gui
      ${CMAKE_BINARY_DIR}/src/auth/maptiler_hmacsha256
    )
    target_link_libraries (authmethod_maptilerhmacsha256 qgis_gui)
    add_dependencies(authmethod_maptilerhmacsha256 ui)
  endif()

  target_compile_definitions(authmethod_maptilerhmacsha256 PRIVATE "-DQT_NO_FOREACH")

  install (TARGETS authmethod_maptilerhmacsha256
    RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
    LIBRARY DESTINATION ${QGIS_PLUGIN_DIR}
  )
endif()
