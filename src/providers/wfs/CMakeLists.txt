
########################################################
# Files

SET(WFS_SRCS
  qgswfsprovider.cpp
  qgswfscapabilities.cpp
  qgswfsdataitems.cpp
  qgswfsfeatureiterator.cpp
  qgswfssourceselect.cpp
)

SET (WFS_MOC_HDRS
  qgswfscapabilities.h
  qgswfsdataitems.h
  qgswfsprovider.h
  qgswfssourceselect.h
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
)

ADD_LIBRARY (wfsprovider MODULE ${WFS_SRCS} ${WFS_MOC_SRCS})

TARGET_LINK_LIBRARIES (wfsprovider
  ${EXPAT_LIBRARY}
  qgis_core
  qgis_gui
)


########################################################
# Install
                
INSTALL(TARGETS wfsprovider
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})
