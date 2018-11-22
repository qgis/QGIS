INCLUDE_DIRECTORIES(
  ${CMAKE_SOURCE_DIR}/src/core
  ${CMAKE_SOURCE_DIR}/src/core/auth
  ${CMAKE_SOURCE_DIR}/src/core/expression
  ${CMAKE_SOURCE_DIR}/src/core/geometry
  ${CMAKE_SOURCE_DIR}/src/core/raster
  ${CMAKE_SOURCE_DIR}/src/core/metadata
  ${CMAKE_SOURCE_DIR}/src/core/symbology
  ${CMAKE_SOURCE_DIR}/src/gui
  ${CMAKE_SOURCE_DIR}/src/gui/auth
  ${CMAKE_BINARY_DIR}/src/ui
  ${CMAKE_BINARY_DIR}/src/core
  ${CMAKE_BINARY_DIR}/src/gui
)

INCLUDE_DIRECTORIES(SYSTEM
  ${QCA_INCLUDE_DIR}
  ${QTKEYCHAIN_INCLUDE_DIR}
)

IF (WITH_GUI)
  INCLUDE_DIRECTORIES(SYSTEM
    ${QSCINTILLA_INCLUDE_DIR}
  )
ENDIF ()

###############################################################################

SET (AFS_SRCS
  qgsarcgisrestutils.cpp
  qgsafsdataitems.cpp
  qgsafsfeatureiterator.cpp
  qgsafsprovider.cpp
  qgsafsproviderextern.cpp
  qgsafsshareddata.cpp
)
SET (AFS_MOC_HDRS
  qgsarcgisrestutils.h
  qgsafsdataitems.h
  qgsafsprovider.h
  qgsafsshareddata.h
)

IF (WITH_GUI)
  SET(AFS_SRCS ${AFS_SRCS}
    qgsafssourceselect.cpp
    qgsarcgisservicesourceselect.cpp
  )
  SET(AFS_MOC_HDRS ${AFS_MOC_HDRS}
    qgsafssourceselect.h
    qgsarcgisservicesourceselect.h
  )
ENDIF ()

QT5_WRAP_CPP (AFS_MOC_SRCS ${AFS_MOC_HDRS})

ADD_LIBRARY (arcgisfeatureserverprovider_a STATIC ${AFS_SRCS} ${AFS_MOC_SRCS})
ADD_LIBRARY(arcgisfeatureserverprovider MODULE ${AFS_SRCS} ${AFS_MOC_SRCS})

TARGET_LINK_LIBRARIES(arcgisfeatureserverprovider
  qgis_core
  ${QCA_LIBRARY}
)

TARGET_LINK_LIBRARIES (arcgisfeatureserverprovider_a
  qgis_core
  ${QCA_LIBRARY}
)

IF (WITH_GUI)
  TARGET_LINK_LIBRARIES(arcgisfeatureserverprovider
    qgis_gui
  )
  TARGET_LINK_LIBRARIES(arcgisfeatureserverprovider_a
    qgis_gui
  )
ENDIF ()

INSTALL (TARGETS arcgisfeatureserverprovider
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})

###############################################################################

SET (AMS_SRCS
  qgsarcgisrestutils.cpp
  qgsamsdataitems.cpp
  qgsamsprovider.cpp
  qgsamsproviderextern.cpp
)
SET (AMS_MOC_HDRS
  qgsarcgisrestutils.h
  qgsamsdataitems.h
  qgsamsprovider.h
)

IF (WITH_GUI)
  SET(AMS_SRCS ${AMS_SRCS}
    qgsamssourceselect.cpp
    qgsarcgisservicesourceselect.cpp
  )
  SET(AMS_MOC_HDRS ${AMS_MOC_HDRS}
    qgsamssourceselect.h
    qgsarcgisservicesourceselect.h
  )
ENDIF ()

QT5_WRAP_CPP (AMS_MOC_SRCS ${AMS_MOC_HDRS})

ADD_LIBRARY(arcgismapserverprovider MODULE ${AMS_SRCS} ${AMS_MOC_SRCS})

TARGET_LINK_LIBRARIES(arcgismapserverprovider
  qgis_core
  ${QCA_LIBRARY}
)

IF (WITH_GUI)
  TARGET_LINK_LIBRARIES(arcgismapserverprovider
    qgis_gui
  )
ENDIF ()

INSTALL (TARGETS arcgismapserverprovider
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})
