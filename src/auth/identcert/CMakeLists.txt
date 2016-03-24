SET(AUTH_IDENTCERT_SRCS
  qgsauthidentcertmethod.cpp
  qgsauthidentcertedit.cpp
)

SET(AUTH_IDENTCERT_HDRS
  qgsauthidentcertmethod.h
  qgsauthidentcertedit.h
)

SET(AUTH_IDENTCERT_MOC_HDRS
  qgsauthidentcertmethod.h
  qgsauthidentcertedit.h
)

SET(AUTH_IDENTCERT_UIS qgsauthidentcertedit.ui)

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

QT4_WRAP_UI (AUTH_IDENTCERT_UIS_H ${AUTH_IDENTCERT_UIS})

QT4_WRAP_CPP(AUTH_IDENTCERT_MOC_SRCS ${AUTH_IDENTCERT_MOC_HDRS})

ADD_LIBRARY (identcertauthmethod MODULE ${AUTH_IDENTCERT_SRCS} ${AUTH_IDENTCERT_HDRS} ${AUTH_IDENTCERT_MOC_SRCS} ${AUTH_IDENTCERT_UIS_H})

TARGET_LINK_LIBRARIES (identcertauthmethod
  qgis_core
  qgis_gui
)

INSTALL(TARGETS identcertauthmethod
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})
