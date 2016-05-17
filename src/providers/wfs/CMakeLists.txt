
########################################################
# Files

SET(WFS_SRCS
  qgswfsprovider.cpp
  qgswfscapabilities.cpp
  qgswfsdataitems.cpp
  qgswfsfeatureiterator.cpp
  qgswfssourceselect.cpp
  qgswfsrequest.cpp
  qgswfsconnection.cpp
  qgswfsdatasourceuri.cpp
  qgswfsconstants.cpp
  qgswfsdescribefeaturetype.cpp
  qgswfsshareddata.cpp
  qgswfstransactionrequest.cpp
  qgswfsutils.cpp
)

SET (WFS_MOC_HDRS
  qgswfscapabilities.h
  qgswfsdataitems.h
  qgswfsprovider.h
  qgswfsfeatureiterator.h
  qgswfssourceselect.h
  qgswfsrequest.h
  qgswfsdescribefeaturetype.h
  qgswfstransactionrequest.h
  qgswfsshareddata.h
  qgswfsutils.h
)

########################################################
# Build

QT4_WRAP_CPP(WFS_MOC_SRCS ${WFS_MOC_HDRS})

INCLUDE_DIRECTORIES (
  ../../core
  ../../core/auth
  ../../core/geometry
  ../../core/symbology-ng  # needed by qgsvectorfilewriter.h
  ../../gui
  ../../gui/auth
  ${CMAKE_CURRENT_BINARY_DIR}/../../ui
)
INCLUDE_DIRECTORIES(SYSTEM
  ${GEOS_INCLUDE_DIR}
  ${GEOS_INCLUDE_DIR}/geos
  ${EXPAT_INCLUDE_DIR}
  ${QSCINTILLA_INCLUDE_DIR}
  ${QCA_INCLUDE_DIR}
  ${GDAL_INCLUDE_DIR} # needed by qgsvectorfilewriter.h
  ${SQLITE3_INCLUDE_DIR}
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
