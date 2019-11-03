SET(AUTH_BASIC_SRCS
  qgsauthbasicmethod.cpp
  qgsauthbasicedit.cpp
)

SET(AUTH_BASIC_HDRS
  qgsauthbasicedit.h
  qgsauthbasicmethod.h
)

SET(AUTH_BASIC_UIS qgsauthbasicedit.ui)

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

QT5_WRAP_UI (AUTH_BASIC_UIS_H ${AUTH_BASIC_UIS})

ADD_LIBRARY (basicauthmethod MODULE ${AUTH_BASIC_SRCS} ${AUTH_BASIC_HDRS} ${AUTH_BASIC_UIS_H})

TARGET_LINK_LIBRARIES (basicauthmethod
  qgis_core
  qgis_gui
)
TARGET_COMPILE_DEFINITIONS(basicauthmethod PRIVATE "-DQT_NO_FOREACH")

INSTALL(TARGETS basicauthmethod
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})
