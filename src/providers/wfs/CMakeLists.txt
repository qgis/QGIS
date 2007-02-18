
########################################################
# Files

SET(WFS_SRCS qgswfsprovider.cpp qgswfsdata.cpp)

SET (WFS_MOC_HDRS
     qgswfsdata.h
)

MESSAGE("-Werror removed for this directory for now - please get rid of all warnings!")
REMOVE_DEFINITIONS( -Werror )

########################################################
# Build

QT4_WRAP_CPP(WFS_MOC_SRCS ${WFS_MOC_HDRS})

INCLUDE_DIRECTORIES (
  ../../core
  ${GEOS_INCLUDE_DIR}
  ${GEOS_INCLUDE_DIR}/geos
  ${EXPAT_INCLUDE_DIR}
)

ADD_LIBRARY (wfsprovider MODULE ${WFS_SRCS} ${WFS_MOC_SRCS})

TARGET_LINK_LIBRARIES (wfsprovider
  ${QT_LIBRARIES}
  ${GEOS_LIBRARY}
  ${EXPAT_LIBRARY}
  qgis_core
)


########################################################
# Install
                
INSTALL(TARGETS wfsprovider
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})
