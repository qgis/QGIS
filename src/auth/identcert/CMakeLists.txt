SET(AUTH_IDENTCERT_SRCS
  qgsauthidentcertmethod.cpp
  qgsauthidentcertedit.cpp
)

SET(AUTH_IDENTCERT_HDRS
  qgsauthidentcertedit.h
  qgsauthidentcertmethod.h
)

SET(AUTH_IDENTCERT_UIS qgsauthidentcertedit.ui)

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

QT5_WRAP_UI (AUTH_IDENTCERT_UIS_H ${AUTH_IDENTCERT_UIS})

ADD_LIBRARY (identcertauthmethod MODULE ${AUTH_IDENTCERT_SRCS} ${AUTH_IDENTCERT_HDRS} ${AUTH_IDENTCERT_UIS_H})

TARGET_LINK_LIBRARIES (identcertauthmethod
  qgis_core
  qgis_gui
)
TARGET_COMPILE_DEFINITIONS(identcertauthmethod PRIVATE "-DQT_NO_FOREACH")

INSTALL(TARGETS identcertauthmethod
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})
