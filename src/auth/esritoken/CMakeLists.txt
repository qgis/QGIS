set(AUTH_ESRITOKEN_SRCS
  core/qgsauthesritokenmethod.cpp
)

set(AUTH_ESRITOKEN_HDRS
  core/qgsauthesritokenmethod.h
)

set(AUTH_ESRITOKEN_UIS_H "")

if (WITH_GUI)
  set(AUTH_ESRITOKEN_SRCS ${AUTH_ESRITOKEN_SRCS}
    gui/qgsauthesritokenmethodgui.cpp
    gui/qgsauthesritokenedit.cpp
  )
  set(AUTH_ESRITOKEN_HDRS ${AUTH_ESRITOKEN_HDRS}
    gui/qgsauthesritokenedit.h
  )
  set(AUTH_ESRITOKEN_UIS gui/qgsauthesritokenedit.ui)
  QT5_WRAP_UI(AUTH_ESRITOKEN_UIS_H ${AUTH_ESRITOKEN_UIS})
endif()

add_library(esritokenauthmethod MODULE ${AUTH_ESRITOKEN_SRCS} ${AUTH_ESRITOKEN_HDRS} ${AUTH_ESRITOKEN_UIS_H})

target_include_directories(esritokenauthmethod PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/core
)

# require c++17
target_compile_features(esritokenauthmethod PRIVATE cxx_std_17)

target_link_libraries(esritokenauthmethod qgis_core)

if (WITH_GUI)
  target_include_directories(esritokenauthmethod PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/gui
    ${CMAKE_CURRENT_BINARY_DIR}
  )
  target_link_libraries(esritokenauthmethod qgis_gui)
endif()

target_compile_definitions(esritokenauthmethod PRIVATE "-DQT_NO_FOREACH")

install(TARGETS esritokenauthmethod
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR}
)
