SET(AUTH_PKIPATHS_SRCS
  qgsauthpkipathsmethod.cpp
  qgsauthpkipathsedit.cpp
)

SET(AUTH_PKIPATHS_HDRS
  qgsauthpkipathsedit.h
  qgsauthpkipathsmethod.h
)

SET(AUTH_PKIPATHS_UIS qgsauthpkipathsedit.ui)

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

QT5_WRAP_UI (AUTH_PKIPATHS_UIS_H ${AUTH_PKIPATHS_UIS})

ADD_LIBRARY (pkipathsauthmethod MODULE ${AUTH_PKIPATHS_SRCS} ${AUTH_PKIPATHS_HDRS} ${AUTH_PKIPATHS_UIS_H})

TARGET_LINK_LIBRARIES (pkipathsauthmethod
  qgis_core
  qgis_gui
)
TARGET_COMPILE_DEFINITIONS(pkipathsauthmethod PRIVATE "-DQT_NO_FOREACH")

INSTALL(TARGETS pkipathsauthmethod
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})
