set(AUTH_OAUTH2_GUI_SRCS
  qgsauthoauth2methodgui.cpp
  qgsauthoauth2edit.cpp
)

set(AUTH_OAUTH2_GUI_HDRS
  qgsauthoauth2edit.h
)

set(AUTH_OAUTH2_GUI_UIS qgsauthoauth2edit.ui)
set(AUTH_OAUTH2_GUI_RCCS ../resources/oauth2_resources.qrc)


QT5_WRAP_UI(AUTH_OAUTH2_GUI_UIS_H ${AUTH_OAUTH2_GUI_UIS})

set(_library_suffix_MODULE "")
set(_library_suffix_STATIC "_static")

foreach(_library_type MODULE STATIC)
  set(_library_name "oauth2authmethodgui${_library_suffix_${_library_type}}")

  add_library(${_library_name} ${_library_type} ${AUTH_OAUTH2_GUI_SRCS} ${AUTH_OAUTH2_GUI_HDRS} ${AUTH_OAUTH2_GUI_UIS_H} ${AUTH_OAUTH2_GUI_RCCS})

  target_include_directories(${_library_name} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../core
    ${CMAKE_CURRENT_BINARY_DIR}
  )

  # require c++17
  target_compile_features(${_library_name} PRIVATE cxx_std_17)

  target_link_libraries (${_library_name}
      qgis_gui
  )

  target_compile_definitions(${_library_name} PRIVATE "-DQT_NO_FOREACH")

  install(TARGETS ${_library_name}
    RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
    LIBRARY DESTINATION ${QGIS_PLUGIN_DIR}
  )
endforeach()
