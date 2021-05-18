set(AUTH_ESRI_TOKEN_SRCS
  qgsauthesritokenmethod.cpp
)
if (WITH_GUI)
  set(AUTH_ESRI_TOKEN_SRCS ${AUTH_ESRI_TOKEN_SRCS}
    qgsauthesritokenmethodgui.cpp
    qgsauthesritokenedit.cpp
  )
endif()

set(AUTH_ESRI_TOKEN_HDRS
  qgsauthesritokenmethod.h
)
if (WITH_GUI)
  set(AUTH_ESRI_TOKEN_HDRS ${AUTH_ESRI_TOKEN_HDRS}
    qgsauthesritokenedit.h
  )
endif()

include_directories (
  ${CMAKE_CURRENT_BINARY_DIR}
)

if (WITH_GUI)
  set(AUTH_ESRI_TOKEN_UIS qgsauthesritokenedit.ui)
  QT5_WRAP_UI(AUTH_ESRI_TOKEN_UIS_H ${AUTH_ESRI_TOKEN_UIS})
  add_library(esritokenauthmethod MODULE ${AUTH_ESRI_TOKEN_SRCS} ${AUTH_ESRI_TOKEN_HDRS} ${AUTH_ESRI_TOKEN_UIS_H})
else()
  add_library(esritokenauthmethod MODULE ${AUTH_ESRI_TOKEN_SRCS} ${AUTH_ESRI_TOKEN_HDRS})
endif()

# require c++17
target_compile_features(esritokenauthmethod PRIVATE cxx_std_17)

target_link_libraries(esritokenauthmethod
  qgis_core
)

if (WITH_GUI)
  target_link_libraries(esritokenauthmethod
    qgis_gui
  )
endif()

target_compile_definitions(esritokenauthmethod PRIVATE "-DQT_NO_FOREACH")

install(TARGETS esritokenauthmethod
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})
