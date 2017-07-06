INCLUDE_DIRECTORIES(
  ${CMAKE_SOURCE_DIR}/src/core
  ${CMAKE_SOURCE_DIR}/src/core/auth
  ${CMAKE_SOURCE_DIR}/src/core/expression
  ${CMAKE_SOURCE_DIR}/src/core/geometry
  ${CMAKE_SOURCE_DIR}/src/core/raster
  ${CMAKE_SOURCE_DIR}/src/core/metadata
  ${CMAKE_SOURCE_DIR}/src/gui
  ${CMAKE_SOURCE_DIR}/src/gui/auth
  ${CMAKE_BINARY_DIR}/src/ui
  ${CMAKE_BINARY_DIR}/src/core
  ${CMAKE_BINARY_DIR}/src/gui
)

INCLUDE_DIRECTORIES(SYSTEM
  ${GEOS_INCLUDE_DIR}
  ${QSCINTILLA_INCLUDE_DIR}
)

###############################################################################

SET (AFS_SRCS
  qgsarcgisrestutils.cpp
  qgsafsdataitems.cpp
  qgsafsfeatureiterator.cpp
  qgsafsprovider.cpp
  qgsafsproviderextern.cpp
  qgsafssourceselect.cpp
  qgsafsshareddata.cpp
)
SET (AFS_MOC_HDRS
  qgsarcgisrestutils.h
  qgsafsdataitems.h
  qgsafsprovider.h
  qgsafssourceselect.h
  qgsafsshareddata.h
)

QT5_WRAP_CPP (AFS_MOC_SRCS ${AFS_MOC_HDRS})

ADD_LIBRARY(arcgisfeatureserverprovider MODULE ${AFS_SRCS} ${AFS_MOC_SRCS})

TARGET_LINK_LIBRARIES(arcgisfeatureserverprovider
  qgis_core
  qgis_gui
)

INSTALL (TARGETS arcgisfeatureserverprovider
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})

###############################################################################

SET (AMS_SRCS
  qgsarcgisrestutils.cpp
  qgsamsdataitems.cpp
  qgsamsprovider.cpp
  qgsamsproviderextern.cpp
  qgsamssourceselect.cpp
)
SET (AMS_MOC_HDRS
  qgsarcgisrestutils.h
  qgsamsdataitems.h
  qgsamsprovider.h
  qgsamssourceselect.h
)

QT5_WRAP_CPP (AMS_MOC_SRCS ${AMS_MOC_HDRS})

ADD_LIBRARY(arcgismapserverprovider MODULE ${AMS_SRCS} ${AMS_MOC_SRCS})

TARGET_LINK_LIBRARIES(arcgismapserverprovider
  qgis_core
  qgis_gui
)

INSTALL (TARGETS arcgismapserverprovider
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})
