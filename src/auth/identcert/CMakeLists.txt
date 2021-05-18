set(AUTH_IDENTCERT_SRCS
  qgsauthidentcertmethod.cpp
)
if (WITH_GUI)
  set(AUTH_IDENTCERT_SRCS ${AUTH_IDENTCERT_SRCS}
    qgsauthidentcertmethodgui.cpp
    qgsauthidentcertedit.cpp
  )
endif()

set(AUTH_IDENTCERT_HDRS
  qgsauthidentcertmethod.h
)
if (WITH_GUI)
  set(AUTH_IDENTCERT_HDRS ${AUTH_IDENTCERT_HDRS}
    qgsauthidentcertedit.h
  )
endif()

include_directories (SYSTEM
  ${QCA_INCLUDE_DIR}
  ${QTKEYCHAIN_INCLUDE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
)

if (WITH_GUI)
  set(AUTH_IDENTCERT_UIS qgsauthidentcertedit.ui)
  QT5_WRAP_UI (AUTH_IDENTCERT_UIS_H ${AUTH_IDENTCERT_UIS})
  add_library (identcertauthmethod MODULE ${AUTH_IDENTCERT_SRCS} ${AUTH_IDENTCERT_HDRS} ${AUTH_IDENTCERT_UIS_H})
else()
  add_library (identcertauthmethod MODULE ${AUTH_IDENTCERT_SRCS} ${AUTH_IDENTCERT_HDRS})
endif()

# require c++17
target_compile_features(identcertauthmethod PRIVATE cxx_std_17)

target_link_libraries (identcertauthmethod
  qgis_core
)

if (WITH_GUI)
  target_link_libraries (identcertauthmethod
    qgis_gui
  )
endif()

target_compile_definitions(identcertauthmethod PRIVATE "-DQT_NO_FOREACH")

install(TARGETS identcertauthmethod
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})
