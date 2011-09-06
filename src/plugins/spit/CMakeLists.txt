
########################################################
# Files

SET (SPIT_SRCS
     qgsspit.cpp
     ../../providers/postgres/qgspgnewconnection.cpp
     qgspgutil.cpp
     qgsshapefile.cpp
)

SET (SPIT_PLUGIN_SRCS
     ${SPIT_SRCS}
     qgsspitplugin.cpp
)

SET (SPIT_EXE_SRCS
     ${SPIT_SRCS}
     main.cpp
)

SET (SPIT_UIS
     ../../ui/qgspgnewconnectionbase.ui
     qgsspitbase.ui
     ../../ui/qgsmessageviewer.ui
     ../../ui/qgscredentialdialog.ui
)

SET (SPIT_EXE_MOC_HDRS
     qgsspit.h
     qgsshapefile.h
     ../../providers/postgres/qgspgnewconnection.h
)

SET (SPIT_PLUGIN_MOC_HDRS
     ${SPIT_EXE_MOC_HDRS}
     qgsspitplugin.h
)

SET (SPIT_RCCS  spit.qrc)

########################################################
# Build

QT4_WRAP_UI (SPIT_UIS_H  ${SPIT_UIS})

QT4_WRAP_CPP (SPIT_PLUGIN_MOC_SRCS  ${SPIT_PLUGIN_MOC_HDRS})
QT4_WRAP_CPP (SPIT_EXE_MOC_SRCS  ${SPIT_EXE_MOC_HDRS})

QT4_ADD_RESOURCES(SPIT_RCC_SRCS ${SPIT_RCCS})

ADD_LIBRARY (spitplugin MODULE ${SPIT_PLUGIN_SRCS} ${SPIT_PLUGIN_MOC_SRCS} ${SPIT_RCC_SRCS} ${SPIT_UIS_H})

INCLUDE_DIRECTORIES(
     ${CMAKE_CURRENT_BINARY_DIR}
     ../../core ../../core/raster ../../core/renderer ../../core/symbology
     ../../gui
     ../../ui
     ../../app
     ../../providers/postgres
     ..
     ${POSTGRES_INCLUDE_DIR}
     ${GDAL_INCLUDE_DIR}
)

TARGET_LINK_LIBRARIES(spitplugin
  ${POSTGRES_LIBRARY}
  qgis_core
  qgis_gui
)

IF (PLUGINS_ALSO_BINARIES)

  ADD_EXECUTABLE (spit ${SPIT_EXE_SRCS} ${SPIT_EXE_MOC_SRCS} ${SPIT_RCC_SRCS}  ${SPIT_UIS_H})

  TARGET_LINK_LIBRARIES(spit
    ${POSTGRES_LIBRARY}
    qgis_core
    qgis_gui
  )
ENDIF (PLUGINS_ALSO_BINARIES)

########################################################
# Install

INSTALL(TARGETS spitplugin
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})

IF (PLUGINS_ALSO_BINARIES)
  INSTALL(TARGETS spit RUNTIME DESTINATION ${QGIS_BIN_DIR})
ENDIF (PLUGINS_ALSO_BINARIES)
