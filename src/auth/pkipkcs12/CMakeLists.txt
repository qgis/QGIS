SET(AUTH_PKCS12_SRCS
  qgsauthpkcs12method.cpp
  qgsauthpkcs12edit.cpp
)

SET(AUTH_PKCS12_HDRS
  qgsauthpkcs12method.h
  qgsauthpkcs12edit.h
)

SET(AUTH_PKCS12_MOC_HDRS
  qgsauthpkcs12method.h
  qgsauthpkcs12edit.h
)

SET(AUTH_PKCS12_UIS qgsauthpkcs12edit.ui)

INCLUDE_DIRECTORIES (
  ../../core
  ../../core/auth
  ../../core/geometry
)
INCLUDE_DIRECTORIES (SYSTEM
  ${QCA_INCLUDE_DIR}
)
INCLUDE_DIRECTORIES (
  ../../gui
  ../../gui/auth
  ${CMAKE_CURRENT_BINARY_DIR}
)

QT4_WRAP_UI (AUTH_PKCS12_UIS_H ${AUTH_PKCS12_UIS})

QT4_WRAP_CPP(AUTH_PKCS12_MOC_SRCS ${AUTH_PKCS12_MOC_HDRS})

ADD_LIBRARY (pkcs12authmethod MODULE ${AUTH_PKCS12_SRCS} ${AUTH_PKCS12_HDRS} ${AUTH_PKCS12_MOC_SRCS} ${AUTH_PKCS12_UIS_H})

TARGET_LINK_LIBRARIES (pkcs12authmethod
  qgis_core
  qgis_gui
)

INSTALL(TARGETS pkcs12authmethod
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})
