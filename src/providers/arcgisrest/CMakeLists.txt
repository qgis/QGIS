INCLUDE_DIRECTORIES(
  ${CMAKE_SOURCE_DIR}/src/core
  ${CMAKE_SOURCE_DIR}/src/core/auth
  ${CMAKE_SOURCE_DIR}/src/core/expression
  ${CMAKE_SOURCE_DIR}/src/core/geometry
  ${CMAKE_SOURCE_DIR}/src/core/raster
  ${CMAKE_SOURCE_DIR}/src/core/metadata
  ${CMAKE_SOURCE_DIR}/src/core/symbology
  ${CMAKE_SOURCE_DIR}/src/core/effects
  ${CMAKE_SOURCE_DIR}/src/core/pal
  ${CMAKE_SOURCE_DIR}/src/gui
  ${CMAKE_SOURCE_DIR}/src/gui/auth
  ${CMAKE_SOURCE_DIR}/external
  ${CMAKE_SOURCE_DIR}/external/nlohmann

  ${CMAKE_BINARY_DIR}/src/ui
  ${CMAKE_BINARY_DIR}/src/core
  ${CMAKE_BINARY_DIR}/src/gui

)

INCLUDE_DIRECTORIES(SYSTEM
  ${GEOS_INCLUDE_DIR}
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
  qgsafsshareddata.cpp
)
IF (WITH_GUI)
  SET(AFS_SRCS ${AFS_SRCS}
    qgsafsprovidergui.cpp
    qgsafsdataitemguiprovider.cpp
    qgsafssourceselect.cpp
    qgsarcgisservicesourceselect.cpp
  )
ENDIF ()

ADD_LIBRARY (arcgisfeatureserverprovider_a STATIC ${AFS_SRCS})
ADD_LIBRARY(arcgisfeatureserverprovider MODULE ${AFS_SRCS})

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
  ADD_DEPENDENCIES(arcgisfeatureserverprovider ui)
  ADD_DEPENDENCIES(arcgisfeatureserverprovider_a ui)
ENDIF ()

INSTALL (TARGETS arcgisfeatureserverprovider
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})

###############################################################################

SET (AMS_SRCS
  qgsarcgisrestutils.cpp
  qgsamsdataitems.cpp
  qgsamsprovider.cpp
)

IF (WITH_GUI)
  SET(AMS_SRCS ${AMS_SRCS}
    qgsamsprovidergui.cpp
    qgsamsdataitemguiprovider.cpp
    qgsamssourceselect.cpp
    qgsarcgisservicesourceselect.cpp
  )
ENDIF ()

ADD_LIBRARY(arcgismapserverprovider MODULE ${AMS_SRCS})

TARGET_LINK_LIBRARIES(arcgismapserverprovider
  qgis_core
  ${QCA_LIBRARY}
)

IF (WITH_GUI)
  TARGET_LINK_LIBRARIES(arcgismapserverprovider
    qgis_gui
  )
  ADD_DEPENDENCIES(arcgismapserverprovider ui)
ENDIF ()

INSTALL (TARGETS arcgismapserverprovider
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})
