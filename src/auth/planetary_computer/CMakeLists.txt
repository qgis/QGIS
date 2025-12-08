set(AUTH_MSPC_SRCS
  core/qgsauthplanetarycomputermethod.cpp
)

set(AUTH_MSPC_HDRS
  core/qgsauthplanetarycomputermethod.h
)

set(AUTH_MSPC_UIS_H "")

if (WITH_GUI)
  set(AUTH_MSPC_SRCS ${AUTH_MSPC_SRCS}
    gui/qgsauthplanetarycomputeredit.cpp
  )
  set(AUTH_MSPC_HDRS ${AUTH_MSPC_HDRS}
    gui/qgsauthplanetarycomputeredit.h
  )
  set(AUTH_MSPC_UIS gui/qgsauthplanetarycomputeredit.ui)
  qt_wrap_ui(AUTH_MSPC_UIS_H ${AUTH_MSPC_UIS})
endif()


# static library
add_library(authmethod_planetary_computer_a STATIC ${AUTH_MSPC_SRCS} ${AUTH_MSPC_HDRS} ${AUTH_MSPC_UIS_H})

target_include_directories(authmethod_planetary_computer_a PUBLIC ${CMAKE_SOURCE_DIR}/src/auth/planetary_computer/core)

target_link_libraries(authmethod_planetary_computer_a qgis_core)

if (WITH_GUI)
  target_include_directories(authmethod_planetary_computer_a PRIVATE
    ${CMAKE_SOURCE_DIR}/src/auth/planetary_computer/gui
    ${CMAKE_BINARY_DIR}/src/auth/planetary_computer
  )
  target_link_libraries (authmethod_planetary_computer_a qgis_gui)
endif()

target_compile_definitions(authmethod_planetary_computer_a PRIVATE "-DQT_NO_FOREACH")

if (FORCE_STATIC_LIBS)
  # for (external) mobile apps to be able to pick up provider for linking
  install (TARGETS authmethod_planetary_computer_a ARCHIVE DESTINATION ${QGIS_PLUGIN_DIR})
else()
  # dynamically loaded module
  add_library(authmethod_planetary_computer MODULE ${AUTH_MSPC_SRCS} ${AUTH_MSPC_HDRS} ${AUTH_MSPC_UIS_H})

  target_link_libraries(authmethod_planetary_computer qgis_core)

  if (WITH_GUI)
    target_include_directories(authmethod_planetary_computer PRIVATE
      ${CMAKE_SOURCE_DIR}/src/auth/planetary_computer/gui
      ${CMAKE_BINARY_DIR}/src/auth/planetary_computer
    )
    target_link_libraries (authmethod_planetary_computer qgis_gui)
    add_dependencies(authmethod_planetary_computer ui)
  endif()

  target_compile_definitions(authmethod_planetary_computer PRIVATE "-DQT_NO_FOREACH")

  install (TARGETS authmethod_planetary_computer
    RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
    LIBRARY DESTINATION ${QGIS_PLUGIN_DIR}
  )
endif()
