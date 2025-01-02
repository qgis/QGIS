
########################################################
# Files

set (WMTS_SRCS
  qgswmts.cpp
  qgswmtsutils.cpp
  qgswmtsgetcapabilities.cpp
  qgswmtsgettile.cpp
  qgswmtsgetfeatureinfo.cpp
  qgswmtsparameters.cpp
)

set (WMTS_HDRS
  qgswmtsparameters.h
)

########################################################
# Build

add_library (wmts MODULE ${WMTS_SRCS} ${WMTS_HDRS})

# require c++17
target_compile_features(wmts PRIVATE cxx_std_17)

include_directories(

  ${CMAKE_SOURCE_DIR}/src/server/
  ${CMAKE_SOURCE_DIR}/src/server/services
  ${CMAKE_SOURCE_DIR}/src/server/services/wmts
  ${CMAKE_SOURCE_DIR}/src/server/services/wms

  ${CMAKE_BINARY_DIR}/src/python
  ${CMAKE_BINARY_DIR}/src/analysis
  ${CMAKE_BINARY_DIR}/src/server

  ${CMAKE_CURRENT_BINARY_DIR}

)


target_link_libraries(wmts
  qgis_core
  qgis_server
)


########################################################
# Install

install(TARGETS wmts
    RUNTIME DESTINATION ${QGIS_SERVER_MODULE_DIR}
    LIBRARY DESTINATION ${QGIS_SERVER_MODULE_DIR}
)

