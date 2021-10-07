set(AUTH_APIHEADER_SRCS
  core/qgsauthapiheadermethod.cpp
)

set(AUTH_APIHEADER_HDRS
  core/qgsauthapiheadermethod.h
)

set(AUTH_APIHEADER_UIS_H "")

if (WITH_GUI)
  set(AUTH_APIHEADER_SRCS ${AUTH_APIHEADER_SRCS}
    gui/qgsauthapiheaderedit.cpp
  )
  set(AUTH_APIHEADER_HDRS ${AUTH_APIHEADER_HDRS}
    gui/qgsauthapiheaderedit.h
  )
  set(AUTH_APIHEADER_UIS gui/qgsauthapiheaderedit.ui)
  if (WITH_QT6)
    QT6_WRAP_UI(AUTH_APIHEADER_UIS_H ${AUTH_APIHEADER_UIS})
  else()
    QT5_WRAP_UI(AUTH_APIHEADER_UIS_H ${AUTH_APIHEADER_UIS})
  endif()
endif()


# static library
add_library(authmethod_apiheader_a STATIC ${AUTH_APIHEADER_SRCS} ${AUTH_APIHEADER_HDRS} ${AUTH_APIHEADER_UIS_H})

target_include_directories(authmethod_apiheader_a PUBLIC ${CMAKE_SOURCE_DIR}/src/auth/apiheader/core)

# require c++17
target_compile_features(authmethod_apiheader_a PRIVATE cxx_std_17)

target_link_libraries(authmethod_apiheader_a qgis_core)

if (WITH_GUI)
  target_include_directories(authmethod_apiheader_a PRIVATE
    ${CMAKE_SOURCE_DIR}/src/auth/apiheader/gui
    ${CMAKE_BINARY_DIR}/src/auth/apiheader
  )
  target_link_libraries (authmethod_apiheader_a qgis_gui)
endif()

target_compile_definitions(authmethod_apiheader_a PRIVATE "-DQT_NO_FOREACH")

if (FORCE_STATIC_LIBS)
  # for (external) mobile apps to be able to pick up provider for linking
  install (TARGETS authmethod_apiheader_a ARCHIVE DESTINATION ${QGIS_PLUGIN_DIR})
else()
  # dynamically loaded module
  add_library(authmethod_apiheader MODULE ${AUTH_APIHEADER_SRCS} ${AUTH_APIHEADER_HDRS} ${AUTH_APIHEADER_UIS_H})

  # require c++17
  target_compile_features(authmethod_apiheader PRIVATE cxx_std_17)

  target_link_libraries(authmethod_apiheader qgis_core)

  if (WITH_GUI)
    target_include_directories(authmethod_apiheader PRIVATE
      ${CMAKE_SOURCE_DIR}/src/auth/apiheader/gui
      ${CMAKE_BINARY_DIR}/src/auth/apiheader
    )
    target_link_libraries (authmethod_apiheader qgis_gui)
    add_dependencies(authmethod_apiheader ui)
  endif()

  target_compile_definitions(authmethod_apiheader PRIVATE "-DQT_NO_FOREACH")

  install (TARGETS authmethod_apiheader
    RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
    LIBRARY DESTINATION ${QGIS_PLUGIN_DIR}
  )
endif()
