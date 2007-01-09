
########################################################
# Files

SET(WFS_SRCS qgswfsprovider.cpp)


########################################################
# Build

QT4_WRAP_CPP(WFS_MOC_SRCS ${WFS_MOC_HDRS})

INCLUDE_DIRECTORIES (
  ../../core
  ${GEOS_INCLUDE_DIR}
  ${GEOS_INCLUDE_DIR}/geos
)

ADD_LIBRARY (wfsprovider MODULE ${WFS_SRCS})

TARGET_LINK_LIBRARIES (wfsprovider
  ${QT_LIBRARIES}
  qgis_core
)


########################################################
# Install
                
INSTALL(TARGETS wfsprovider
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})
