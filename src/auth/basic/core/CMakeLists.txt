set(AUTH_BASIC_SRCS
  qgsauthbasicmethod.cpp
)

set(AUTH_BASIC_HDRS
  qgsauthbasicmethod.h
)

add_library(basicauthmethod MODULE ${AUTH_BASIC_SRCS} ${AUTH_BASIC_HDRS})

# require c++17
target_compile_features(basicauthmethod PRIVATE cxx_std_17)

target_include_directories(basicauthmethod PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(basicauthmethod
  qgis_core
)

target_compile_definitions(basicauthmethod PRIVATE "-DQT_NO_FOREACH")

install(TARGETS basicauthmethod
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR}
)
