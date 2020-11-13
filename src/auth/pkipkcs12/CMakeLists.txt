set(AUTH_PKCS12_SRCS
  qgsauthpkcs12method.cpp
  qgsauthpkcs12edit.cpp
)

set(AUTH_PKCS12_HDRS
  qgsauthpkcs12edit.h
  qgsauthpkcs12method.h
)

set(AUTH_PKCS12_UIS qgsauthpkcs12edit.ui)

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

QT5_WRAP_UI (AUTH_PKCS12_UIS_H ${AUTH_PKCS12_UIS})

add_library (pkcs12authmethod MODULE ${AUTH_PKCS12_SRCS} ${AUTH_PKCS12_HDRS} ${AUTH_PKCS12_UIS_H})

target_link_libraries (pkcs12authmethod
  qgis_core
  qgis_gui
)
target_compile_definitions(pkcs12authmethod PRIVATE "-DQT_NO_FOREACH")

install(TARGETS pkcs12authmethod
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})
