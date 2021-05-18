set(AUTH_PKCS12_SRCS
  qgsauthpkcs12method.cpp
)

set(AUTH_PKCS12_HDRS
  qgsauthpkcs12method.h
)

add_library(pkcs12authmethod MODULE ${AUTH_PKCS12_SRCS} ${AUTH_PKCS12_HDRS})

# require c++17
target_compile_features(pkcs12authmethod PRIVATE cxx_std_17)

target_include_directories (pkcs12authmethod SYSTEM PUBLIC
  ${QCA_INCLUDE_DIR}
  ${QTKEYCHAIN_INCLUDE_DIR}
)

target_include_directories(pkcs12authmethod PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(pkcs12authmethod
  qgis_core
)

target_compile_definitions(pkcs12authmethod PRIVATE "-DQT_NO_FOREACH")

install(TARGETS pkcs12authmethod
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR}
)
