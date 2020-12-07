set(AUTH_BASIC_SRCS
  qgsauthbasicmethod.cpp
  qgsauthbasicedit.cpp
)

set(AUTH_BASIC_HDRS
  qgsauthbasicedit.h
  qgsauthbasicmethod.h
)

set(AUTH_BASIC_UIS qgsauthbasicedit.ui)

include_directories (
  ../../core
  ../../core/auth
  ../../core/geometry
  ../../core/metadata
  ${CMAKE_BINARY_DIR}/src/core
  ${CMAKE_BINARY_DIR}/src/gui
)
include_directories (SYSTEM
  ${QCA_INCLUDE_DIR}
  ${QTKEYCHAIN_INCLUDE_DIR}
)
include_directories (
  ../../gui
  ../../gui/auth
  ${CMAKE_CURRENT_BINARY_DIR}
)

QT5_WRAP_UI (AUTH_BASIC_UIS_H ${AUTH_BASIC_UIS})

add_library (basicauthmethod MODULE ${AUTH_BASIC_SRCS} ${AUTH_BASIC_HDRS} ${AUTH_BASIC_UIS_H})

target_link_libraries (basicauthmethod
  qgis_core
  qgis_gui
)
target_compile_definitions(basicauthmethod PRIVATE "-DQT_NO_FOREACH")

install(TARGETS basicauthmethod
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})
