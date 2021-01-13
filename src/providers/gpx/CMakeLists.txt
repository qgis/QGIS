
########################################################
# Files

set (GPX_SRCS
  qgsgpxprovider.cpp
  qgsgpxfeatureiterator.cpp
  gpsdata.cpp
)

########################################################
# Build

include_directories(

)
include_directories (SYSTEM
)

add_library(gpxprovider MODULE ${GPX_SRCS})

target_link_libraries(gpxprovider
  ${EXPAT_LIBRARY}
  qgis_core
)


########################################################
# Install

install (TARGETS gpxprovider
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})
