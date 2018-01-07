
########################################################
# Files

SET (wfs_SRCS
  qgswfs.cpp
  qgswfsutils.cpp
  qgswfsgetcapabilities.cpp
  qgswfsgetcapabilities_1_0_0.cpp
  qgswfsdescribefeaturetype.cpp
  qgswfsgetfeature.cpp
  qgswfstransaction.cpp
  qgswfstransaction_1_0_0.cpp
  qgswfsparameters.cpp
)

SET (wfs_MOC_HDRS
  qgswfsparameters.h
)

########################################################
# Build

QT5_WRAP_CPP(wfs_MOC_SRCS ${wfs_MOC_HDRS})

ADD_LIBRARY (wfs MODULE ${wfs_SRCS} ${wfs_MOC_SRCS} ${wfs_MOC_HDRS})


INCLUDE_DIRECTORIES(SYSTEM
  ${GDAL_INCLUDE_DIR}
  ${GEOS_INCLUDE_DIR}
  ${PROJ_INCLUDE_DIR}
  ${POSTGRES_INCLUDE_DIR}
)

INCLUDE_DIRECTORIES(
  ${CMAKE_BINARY_DIR}/src/core
  ${CMAKE_BINARY_DIR}/src/python
  ${CMAKE_BINARY_DIR}/src/analysis
  ${CMAKE_BINARY_DIR}/src/server
  ${CMAKE_CURRENT_BINARY_DIR}
  ../../../core
  ../../../core/dxf
  ../../../core/expression
  ../../../core/geometry
  ../../../core/metadata
  ../../../core/raster
  ../../../core/symbology
  ../../../core/composer
  ../../../core/layertree
  ../..
  ..
  .
)


#endif
TARGET_LINK_LIBRARIES(wfs
  qgis_core
  qgis_server
)


########################################################
# Install

INSTALL(TARGETS wfs
    RUNTIME DESTINATION ${QGIS_SERVER_MODULE_DIR}
    LIBRARY DESTINATION ${QGIS_SERVER_MODULE_DIR}
)

