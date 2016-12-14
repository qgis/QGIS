
########################################################
# Files

SET (dummy_SRCS
  dummy.cpp
)

########################################################
# Build

ADD_LIBRARY (dummy MODULE ${dummy_SRCS})


INCLUDE_DIRECTORIES(
  ${CMAKE_CURRENT_BINARY_DIR}
  ../../../core ../../../core/geometry ../../../core/raster
  ../..
  ..
  .
)

TARGET_LINK_LIBRARIES(dummy
  qgis_core
  qgis_server
)


########################################################
# Install

INSTALL(TARGETS dummy
    RUNTIME DESTINATION ${QGIS_SERVER_MODULE_DIR}
    LIBRARY DESTINATION ${QGIS_SERVER_MODULE_DIR}
)

