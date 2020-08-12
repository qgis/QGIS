SET(AUTH_PKCS12_SRCS
  qgsauthpkcs12method.cpp
  qgsauthpkcs12edit.cpp
)

SET(AUTH_PKCS12_HDRS
  qgsauthpkcs12edit.h
  qgsauthpkcs12method.h
)

SET(AUTH_PKCS12_UIS qgsauthpkcs12edit.ui)

INCLUDE_DIRECTORIES (
  ../../core
  ../../core/auth
  ../../core/geometry
  ../../core/metadata
  ${CMAKE_BINARY_DIR}/src/core
  ${CMAKE_BINARY_DIR}/src/gui
)
INCLUDE_DIRECTORIES (SYSTEM
  ${QCA_INCLUDE_DIR}
  ${QTKEYCHAIN_INCLUDE_DIR}
)
INCLUDE_DIRECTORIES (
  ../../gui
  ../../gui/auth
  ${CMAKE_CURRENT_BINARY_DIR}
)

QT5_WRAP_UI (AUTH_PKCS12_UIS_H ${AUTH_PKCS12_UIS})

ADD_LIBRARY (pkcs12authmethod MODULE ${AUTH_PKCS12_SRCS} ${AUTH_PKCS12_HDRS} ${AUTH_PKCS12_UIS_H})

TARGET_LINK_LIBRARIES (pkcs12authmethod
  qgis_core
  qgis_gui
)
TARGET_COMPILE_DEFINITIONS(pkcs12authmethod PRIVATE "-DQT_NO_FOREACH")

INSTALL(TARGETS pkcs12authmethod
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})
