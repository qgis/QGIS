set(AUTH_IDENTCERT_SRCS
  qgsauthidentcertmethod.cpp
  qgsauthidentcertedit.cpp
)

set(AUTH_IDENTCERT_HDRS
  qgsauthidentcertedit.h
  qgsauthidentcertmethod.h
)

set(AUTH_IDENTCERT_UIS qgsauthidentcertedit.ui)

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

QT5_WRAP_UI (AUTH_IDENTCERT_UIS_H ${AUTH_IDENTCERT_UIS})

add_library (identcertauthmethod MODULE ${AUTH_IDENTCERT_SRCS} ${AUTH_IDENTCERT_HDRS} ${AUTH_IDENTCERT_UIS_H})

# require c++17
target_compile_features(identcertauthmethod PRIVATE cxx_std_17)

target_link_libraries (identcertauthmethod
  qgis_core
  qgis_gui
)
target_compile_definitions(identcertauthmethod PRIVATE "-DQT_NO_FOREACH")

install(TARGETS identcertauthmethod
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})
