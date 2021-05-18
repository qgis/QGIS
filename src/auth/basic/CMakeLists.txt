set(AUTH_BASIC_SRCS
  qgsauthbasicmethod.cpp
)
if (WITH_GUI)
  set(AUTH_BASIC_SRCS ${AUTH_BASIC_SRCS}
    qgsauthbasicmethodgui.cpp
    qgsauthbasicedit.cpp
  )
endif()

set(AUTH_BASIC_HDRS
  qgsauthbasicmethod.h
)
if (WITH_GUI)
  set(AUTH_BASIC_HDRS ${AUTH_BASIC_HDRS}
    qgsauthbasicedit.h
  )
endif()

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
)

if (WITH_GUI)
  set(AUTH_BASIC_UIS qgsauthbasicedit.ui)
  QT5_WRAP_UI(AUTH_BASIC_UIS_H ${AUTH_BASIC_UIS})
  add_library(basicauthmethod MODULE ${AUTH_BASIC_SRCS} ${AUTH_BASIC_HDRS} ${AUTH_BASIC_UIS_H})
else()
  add_library(basicauthmethod MODULE ${AUTH_BASIC_SRCS} ${AUTH_BASIC_HDRS})
endif()

# require c++17
target_compile_features(basicauthmethod PRIVATE cxx_std_17)

target_link_libraries(basicauthmethod
  qgis_core
)

if (WITH_GUI)
  target_link_libraries (basicauthmethod
    qgis_gui
  )
endif()

target_compile_definitions(basicauthmethod PRIVATE "-DQT_NO_FOREACH")

install(TARGETS basicauthmethod
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})
