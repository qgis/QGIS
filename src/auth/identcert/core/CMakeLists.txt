set(AUTH_IDENTCERT_SRCS
  qgsauthidentcertmethod.cpp
)

set(AUTH_IDENTCERT_HDRS
  qgsauthidentcertmethod.h
)

add_library(identcertauthmethod MODULE ${AUTH_IDENTCERT_SRCS} ${AUTH_IDENTCERT_HDRS})

target_include_directories(identcertauthmethod SYSTEM PUBLIC
  ${QCA_INCLUDE_DIR}
  ${QTKEYCHAIN_INCLUDE_DIR}
)

target_include_directories(identcertauthmethod PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
)

# require c++17
target_compile_features(identcertauthmethod PRIVATE cxx_std_17)

target_link_libraries(identcertauthmethod
  qgis_core
)

target_compile_definitions(identcertauthmethod PRIVATE "-DQT_NO_FOREACH")

install(TARGETS identcertauthmethod
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR}
)
