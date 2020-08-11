########################################################
# Files

SET (LANDINGPAGE_SRCS
  ${CMAKE_SOURCE_DIR}/external/nlohmann/json.hpp
  ${CMAKE_SOURCE_DIR}/external/inja/inja.hpp
  qgslandingpage.cpp
  qgslandingpageutils.cpp
  qgslandingpagehandlers.cpp
)

########################################################
# Build

ADD_LIBRARY (landingpage MODULE ${LANDINGPAGE_SRCS})

INCLUDE_DIRECTORIES(SYSTEM
)

INCLUDE_DIRECTORIES(
  ${CMAKE_SOURCE_DIR}/external
  ${CMAKE_SOURCE_DIR}/external/nlohmann

  ${CMAKE_SOURCE_DIR}/src/core
  ${CMAKE_SOURCE_DIR}/src/core/geometry
  ${CMAKE_SOURCE_DIR}/src/core/expression
  ${CMAKE_SOURCE_DIR}/src/core/symbology
  ${CMAKE_SOURCE_DIR}/src/core/metadata
  ${CMAKE_SOURCE_DIR}/src/core/layertree
  ${CMAKE_SOURCE_DIR}/src/server
  ${CMAKE_SOURCE_DIR}/src/server/services
  ${CMAKE_SOURCE_DIR}/src/server/services/landingpage

  ${CMAKE_BINARY_DIR}/src/core
  ${CMAKE_BINARY_DIR}/src/python
  ${CMAKE_BINARY_DIR}/src/server

  ${CMAKE_CURRENT_BINARY_DIR}
)


TARGET_LINK_LIBRARIES(landingpage
  qgis_core
  qgis_server
)


########################################################
# Install

INSTALL(TARGETS landingpage
    RUNTIME DESTINATION ${QGIS_SERVER_MODULE_DIR}
    LIBRARY DESTINATION ${QGIS_SERVER_MODULE_DIR}
)

