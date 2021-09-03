
########################################################
# Files

set (WFS_SRCS
  qgswfs.cpp
  qgswfsutils.cpp
  qgswfsgetcapabilities.cpp
  qgswfsgetcapabilities_1_0_0.cpp
  qgswfsdescribefeaturetype.cpp
  qgswfsgetfeature.cpp
  qgswfstransaction.cpp
  qgswfstransaction_1_0_0.cpp
  qgswfsparameters.cpp
)

set (WFS_HDRS
  qgswfsparameters.h
)

########################################################
# Build

add_library (wfs MODULE ${WFS_SRCS} ${WFS_HDRS})

# require c++17
target_compile_features(wfs PRIVATE cxx_std_17)

include_directories(SYSTEM
  ${GDAL_INCLUDE_DIR}
  ${POSTGRES_INCLUDE_DIR}
)

include_directories(
  ${CMAKE_SOURCE_DIR}/src/server
  ${CMAKE_SOURCE_DIR}/src/server/services
  ${CMAKE_SOURCE_DIR}/src/server/services/wfs

  ${CMAKE_BINARY_DIR}/src/python
  ${CMAKE_BINARY_DIR}/src/analysis
  ${CMAKE_BINARY_DIR}/src/server

  ${CMAKE_CURRENT_BINARY_DIR}
)


target_link_libraries(wfs
  qgis_core
  qgis_server
)


########################################################
# Install

install(TARGETS wfs
    RUNTIME DESTINATION ${QGIS_SERVER_MODULE_DIR}
    LIBRARY DESTINATION ${QGIS_SERVER_MODULE_DIR}
)

