
########################################################
# Files

set (WFS_SRCS
  qgswfs.cpp
  qgswfsutils.cpp
  qgswfsgetcapabilities.cpp
  qgswfsgetcapabilities_1_0_0.cpp
  qgswfsdescribefeaturetype.cpp
  qgswfsdescribefeaturetypegml.cpp
  qgswfsdescribefeaturetypejson.cpp
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

set(_library_suffix_MODULE "")
set(_library_suffix_STATIC "_static")

foreach(_library_type MODULE STATIC)
  set(_library_name "wfs${_library_suffix_${_library_type}}")

  add_library (${_library_name} ${_library_type} ${WFS_SRCS} ${WFS_HDRS})

  # require c++17
  target_compile_features(${_library_name} PRIVATE cxx_std_17)

  target_include_directories(${_library_name} PUBLIC
    ${CMAKE_SOURCE_DIR}/src/server
    ${CMAKE_SOURCE_DIR}/src/server/services
    ${CMAKE_SOURCE_DIR}/src/server/services/wfs

    ${CMAKE_BINARY_DIR}/src/python
    ${CMAKE_BINARY_DIR}/src/analysis
    ${CMAKE_BINARY_DIR}/src/server

    ${CMAKE_CURRENT_BINARY_DIR}
  )

  target_link_libraries(${_library_name}
    qgis_core
    qgis_server
  )
endforeach()


########################################################
# Install

install(TARGETS wfs
    RUNTIME DESTINATION ${QGIS_SERVER_MODULE_DIR}
    LIBRARY DESTINATION ${QGIS_SERVER_MODULE_DIR}
)

