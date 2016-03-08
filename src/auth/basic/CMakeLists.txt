SET(AUTH_BASIC_SRCS
  qgsauthbasicmethod.cpp
  qgsauthbasicedit.cpp
)

SET(AUTH_BASIC_HDRS
  qgsauthbasicmethod.h
  qgsauthbasicedit.h
)

SET(AUTH_BASIC_MOC_HDRS
  qgsauthbasicmethod.h
  qgsauthbasicedit.h
)

SET(AUTH_BASIC_UIS qgsauthbasicedit.ui)

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

QT4_WRAP_UI (AUTH_BASIC_UIS_H ${AUTH_BASIC_UIS})

QT4_WRAP_CPP(AUTH_BASIC_MOC_SRCS ${AUTH_BASIC_MOC_HDRS})

ADD_LIBRARY (basicauthmethod MODULE ${AUTH_BASIC_SRCS} ${AUTH_BASIC_HDRS} ${AUTH_BASIC_MOC_SRCS} ${AUTH_BASIC_UIS_H})

TARGET_LINK_LIBRARIES (basicauthmethod
  qgis_core
  qgis_gui
)

INSTALL(TARGETS basicauthmethod
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})
