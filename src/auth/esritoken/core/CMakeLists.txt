set(AUTH_ESRITOKENTOKEN_SRCS
  qgsauthesritokenmethod.cpp
)

set(AUTH_ESRITOKEN_HDRS
  qgsauthesritokenmethod.h
)

add_library(esritokenauthmethod MODULE ${AUTH_ESRITOKEN_SRCS} ${AUTH_ESRITOKEN_HDRS})

# require c++17
target_compile_features(esritokenauthmethod PRIVATE cxx_std_17)

target_include_directories(esritokenauthmethod PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(esritokenauthmethod
  qgis_core
)

target_compile_definitions(esritokenauthmethod PRIVATE "-DQT_NO_FOREACH")

install(TARGETS esritokenauthmethod
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR}
)
