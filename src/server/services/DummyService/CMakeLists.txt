
########################################################
# Files

set (dummy_SRCS
  dummy.cpp
)

########################################################
# Build

add_library (dummy MODULE ${dummy_SRCS})

# require c++17
target_compile_features(dummy PRIVATE cxx_std_17)

include_directories(
  ${CMAKE_SOURCE_DIR}/src/server
  ${CMAKE_SOURCE_DIR}/src/server/services
  ${CMAKE_SOURCE_DIR}/src/server/services/DummeyService

  ${CMAKE_BINARY_DIR}/src/python
  ${CMAKE_BINARY_DIR}/src/analysis
  ${CMAKE_BINARY_DIR}/src/server
  ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(dummy
  qgis_core
  qgis_server
)


########################################################
# Install

install(TARGETS dummy
    RUNTIME DESTINATION ${QGIS_SERVER_MODULE_DIR}
    LIBRARY DESTINATION ${QGIS_SERVER_MODULE_DIR}
)

