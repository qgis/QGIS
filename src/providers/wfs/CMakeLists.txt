
########################################################
# Files

SET(WFS_SRCS
  ${CMAKE_SOURCE_DIR}/external/nlohmann/json.hpp
  qgswfsprovider.cpp
  qgswfscapabilities.cpp
  qgswfsdataitems.cpp
  qgswfsfeatureiterator.cpp
  qgswfsrequest.cpp
  qgswfsconnection.cpp
  qgswfsdatasourceuri.cpp
  qgswfsconstants.cpp
  qgswfsdescribefeaturetype.cpp
  qgswfsshareddata.cpp
  qgswfstransactionrequest.cpp
  qgswfsutils.cpp
  qgsbackgroundcachedfeatureiterator.cpp
  qgsbackgroundcachedshareddata.cpp
  qgscachedirectorymanager.cpp
  qgsbasenetworkrequest.cpp
  qgsoapiflandingpagerequest.cpp
  qgsoapifapirequest.cpp
  qgsoapifcollection.cpp
  qgsoapifitemsrequest.cpp
  qgsoapifprovider.cpp
  qgsoapifutils.cpp
)

IF (WITH_GUI)
  SET(WFS_SRCS ${WFS_SRCS}
    qgswfsprovidergui.cpp
    qgswfsdataitemguiprovider.cpp
    qgswfssourceselect.cpp
    qgswfsnewconnection.cpp
    qgswfsguiutils.cpp
  )
ENDIF ()

########################################################
# Build

INCLUDE_DIRECTORIES (
  ${CMAKE_SOURCE_DIR}/src/core
  ${CMAKE_SOURCE_DIR}/src/core/auth
  ${CMAKE_SOURCE_DIR}/src/core/expression
  ${CMAKE_SOURCE_DIR}/src/core/geometry
  ${CMAKE_SOURCE_DIR}/src/core/symbology  # needed by qgsvectorfilewriter.h
  ${CMAKE_SOURCE_DIR}/src/core/effects    # needed by qgssymbollayer.h
  ${CMAKE_SOURCE_DIR}/src/core/geocms/geonode
  ${CMAKE_SOURCE_DIR}/src/core/metadata
  ${CMAKE_SOURCE_DIR}/src/gui
  ${CMAKE_SOURCE_DIR}/src/gui/auth
  ${CMAKE_SOURCE_DIR}/external
  ${CMAKE_SOURCE_DIR}/external/nlohmann

  ${CMAKE_BINARY_DIR}/src/core
  ${CMAKE_BINARY_DIR}/src/gui
  ${CMAKE_BINARY_DIR}/src/ui
)
INCLUDE_DIRECTORIES(SYSTEM
  ${EXPAT_INCLUDE_DIR}
  ${QCA_INCLUDE_DIR}
  ${QTKEYCHAIN_INCLUDE_DIR}
  ${GDAL_INCLUDE_DIR} # needed by qgsvectorfilewriter.h
  ${SQLITE3_INCLUDE_DIR}
  ${GDAL_INCLUDE_DIR}
)

IF (WITH_GUI)
  INCLUDE_DIRECTORIES(SYSTEM
    ${QSCINTILLA_INCLUDE_DIR}
  )
ENDIF ()

ADD_LIBRARY (wfsprovider MODULE ${WFS_SRCS})

TARGET_LINK_LIBRARIES (wfsprovider
  ${EXPAT_LIBRARY}
  qgis_core
)

IF (WITH_GUI)
  TARGET_LINK_LIBRARIES (wfsprovider
    qgis_gui
  )
  ADD_DEPENDENCIES(wfsprovider ui)
ENDIF ()

########################################################
# Install

INSTALL(TARGETS wfsprovider
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})
