set(AUTH_PKCS12_SRCS
  qgsauthpkcs12method.cpp
)
if (WITH_GUI)
  set(AUTH_PKCS12_SRCS ${AUTH_PKCS12_SRCS}
    qgsauthpkcs12methodgui.cpp
    qgsauthpkcs12edit.cpp
  )
endif()

set(AUTH_PKCS12_HDRS
  qgsauthpkcs12method.h
)
if (WITH_GUI)
  set(AUTH_PKCS12_HDRS ${AUTH_PKCS12_HDRS}
    qgsauthpkcs12edit.h
  )
endif()

include_directories (SYSTEM
  ${QCA_INCLUDE_DIR}
  ${QTKEYCHAIN_INCLUDE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
)

if (WITH_GUI)
  set(AUTH_PKCS12_UIS qgsauthpkcs12edit.ui)
  QT5_WRAP_UI (AUTH_PKCS12_UIS_H ${AUTH_PKCS12_UIS})
  add_library (pkcs12authmethod MODULE ${AUTH_PKCS12_SRCS} ${AUTH_PKCS12_HDRS} ${AUTH_PKCS12_UIS_H})
else()
  add_library (pkcs12authmethod MODULE ${AUTH_PKCS12_SRCS} ${AUTH_PKCS12_HDRS})
endif()

# require c++17
target_compile_features(pkcs12authmethod PRIVATE cxx_std_17)

target_link_libraries (pkcs12authmethod
  qgis_core
)

if (WITH_GUI)
  target_link_libraries (pkcs12authmethod
    qgis_gui
  )
endif()

target_compile_definitions(pkcs12authmethod PRIVATE "-DQT_NO_FOREACH")

install(TARGETS pkcs12authmethod
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})
