########################################################
# Files

set (WFS3_SRCS
  ${CMAKE_SOURCE_DIR}/external/inja/inja.hpp
  qgswfs3.cpp
  qgswfs3handlers.cpp
)

########################################################
# Build

add_library (wfs3 MODULE ${WFS3_SRCS})

# require c++17
target_compile_features(wfs3 PRIVATE cxx_std_17)

include_directories(

  ${CMAKE_SOURCE_DIR}/src/server
  ${CMAKE_SOURCE_DIR}/src/server/services
  ${CMAKE_SOURCE_DIR}/src/server/services/wfs3

  ${CMAKE_BINARY_DIR}/src/python
  ${CMAKE_BINARY_DIR}/src/analysis
  ${CMAKE_BINARY_DIR}/src/server

  ${CMAKE_CURRENT_BINARY_DIR}
)


target_link_libraries(wfs3
  qgis_core
  qgis_server
)


########################################################
# Install

install(TARGETS wfs3
    RUNTIME DESTINATION ${QGIS_SERVER_MODULE_DIR}
    LIBRARY DESTINATION ${QGIS_SERVER_MODULE_DIR}
)

