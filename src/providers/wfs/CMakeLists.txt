
########################################################
# Files

SET(WFS_SRCS qgswfsprovider.cpp)

SET (WFS_MOC_HDRS ""
)

########################################################
# Build

QT4_WRAP_CPP(WFS_MOC_SRCS ${WFS_MOC_HDRS})

INCLUDE_DIRECTORIES (
  ../../core
  ../../gui
  ${CMAKE_CURRENT_BINARY_DIR}/../../ui
  ${GEOS_INCLUDE_DIR}
  ${GEOS_INCLUDE_DIR}/geos
  ${EXPAT_INCLUDE_DIR}
  ${GDAL_INCLUDE_DIR}
)

ADD_LIBRARY (wfsprovider MODULE ${WFS_SRCS} ${WFS_MOC_SRCS})

TARGET_LINK_LIBRARIES (wfsprovider
  ${QT_LIBRARIES}
  ${GEOS_LIBRARY}
  ${EXPAT_LIBRARY}
  ${GDAL_LIBRARIES}
  qgis_core
)


########################################################
# Install
                
INSTALL(TARGETS wfsprovider
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})
