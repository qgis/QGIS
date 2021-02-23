set(AUTH_PKIPATHS_SRCS
  qgsauthpkipathsmethod.cpp
  qgsauthpkipathsedit.cpp
)

set(AUTH_PKIPATHS_HDRS
  qgsauthpkipathsedit.h
  qgsauthpkipathsmethod.h
)

set(AUTH_PKIPATHS_UIS qgsauthpkipathsedit.ui)

include_directories (
  ../../core
  ../../core/auth
  ../../core/geometry
  ../../core/metadata
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

QT5_WRAP_UI (AUTH_PKIPATHS_UIS_H ${AUTH_PKIPATHS_UIS})

add_library (pkipathsauthmethod MODULE ${AUTH_PKIPATHS_SRCS} ${AUTH_PKIPATHS_HDRS} ${AUTH_PKIPATHS_UIS_H})

# require c++17
target_compile_features(pkipathsauthmethod PRIVATE cxx_std_17)

target_link_libraries (pkipathsauthmethod
  qgis_core
  qgis_gui
)
target_compile_definitions(pkipathsauthmethod PRIVATE "-DQT_NO_FOREACH")

install(TARGETS pkipathsauthmethod
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})
