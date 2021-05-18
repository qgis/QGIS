set(AUTH_PKIPATHS_SRCS
  qgsauthpkipathsmethod.cpp
)

set(AUTH_PKIPATHS_HDRS
  qgsauthpkipathsmethod.h
)

add_library(pkipathsauthmethod MODULE ${AUTH_PKIPATHS_SRCS} ${AUTH_PKIPATHS_HDRS})

# require c++17
target_compile_features(pkipathsauthmethod PRIVATE cxx_std_17)

target_include_directories (pkipathsauthmethod SYSTEM PUBLIC
  ${QCA_INCLUDE_DIR}
  ${QTKEYCHAIN_INCLUDE_DIR}
)

target_include_directories(pkipathsauthmethod PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(pkipathsauthmethod
  qgis_core
)

target_compile_definitions(pkipathsauthmethod PRIVATE "-DQT_NO_FOREACH")

install(TARGETS pkipathsauthmethod
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR}
)
