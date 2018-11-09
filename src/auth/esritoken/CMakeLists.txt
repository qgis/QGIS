SET(AUTH_ESRI_TOKEN_SRCS
  qgsauthesritokenmethod.cpp
  qgsauthesritokenedit.cpp
)

SET(AUTH_ESRI_TOKEN_HDRS
  qgsauthesritokenmethod.h
  qgsauthesritokenedit.h
)

SET(AUTH_ESRI_TOKEN_MOC_HDRS
  qgsauthesritokenmethod.h
  qgsauthesritokenedit.h
)

SET(AUTH_ESRI_TOKEN_UIS qgsauthesritokenedit.ui)

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

QT5_WRAP_UI (AUTH_ESRI_TOKEN_UIS_H ${AUTH_ESRI_TOKEN_UIS})

QT5_WRAP_CPP(AUTH_ESRI_TOKEN_MOC_SRCS ${AUTH_ESRI_TOKEN_MOC_HDRS})

ADD_LIBRARY (esritokenauthmethod MODULE ${AUTH_ESRI_TOKEN_SRCS} ${AUTH_ESRI_TOKEN_HDRS} ${AUTH_ESRI_TOKEN_MOC_SRCS} ${AUTH_ESRI_TOKEN_UIS_H})

TARGET_LINK_LIBRARIES (esritokenauthmethod
  qgis_core
  qgis_gui
)

INSTALL(TARGETS esritokenauthmethod
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})
