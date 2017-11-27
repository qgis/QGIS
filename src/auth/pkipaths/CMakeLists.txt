SET(AUTH_PKIPATHS_SRCS
  qgsauthpkipathsmethod.cpp
  qgsauthpkipathsedit.cpp
)

SET(AUTH_PKIPATHS_HDRS
  qgsauthpkipathsmethod.h
  qgsauthpkipathsedit.h
)

SET(AUTH_PKIPATHS_MOC_HDRS
  qgsauthpkipathsmethod.h
  qgsauthpkipathsedit.h
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

QT5_WRAP_CPP(AUTH_PKIPATHS_MOC_SRCS ${AUTH_PKIPATHS_MOC_HDRS})

ADD_LIBRARY (pkipathsauthmethod MODULE ${AUTH_PKIPATHS_SRCS} ${AUTH_PKIPATHS_HDRS} ${AUTH_PKIPATHS_MOC_SRCS} ${AUTH_PKIPATHS_UIS_H})

TARGET_LINK_LIBRARIES (pkipathsauthmethod
  qgis_core
  qgis_gui
)

INSTALL(TARGETS pkipathsauthmethod
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})
