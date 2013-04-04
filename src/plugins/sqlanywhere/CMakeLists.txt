########################################################
# Files

SET (sqlanywhere_SRCS
     sqlanywhere.cpp
     sasourceselect.cpp
     sanewconnection.cpp
     sadbtablemodel.cpp
)

SET (sqlanywhere_UIS
    sanewconnectionbase.ui
    sasourceselectbase.ui
)

SET (sqlanywhere_MOC_HDRS
     sqlanywhere.h
     sasourceselect.h
     sanewconnection.h
     salayer.h
     sadbtablemodel.h
)

SET (sqlanywhere_RCCS  sqlanywhere.qrc)

IF (WIN32)
  IF (MSVC)
    ADD_DEFINITIONS("-DSACONN_EXPORT=__declspec(dllexport)")
  ELSE (MSVC)
    ADD_DEFINITIONS("-USACONN_EXPORT \"-DSACONN_EXPORT=__declspec(dllexport)\"")
  ENDIF (MSVC)
ELSE (WIN32)
  ADD_DEFINITIONS(-DSACONN_EXPORT=)
ENDIF (WIN32)

########################################################
# Build

QT4_WRAP_UI (sqlanywhere_UIS_H  ${sqlanywhere_UIS})

QT4_WRAP_CPP (sqlanywhere_MOC_SRCS  ${sqlanywhere_MOC_HDRS})

QT4_ADD_RESOURCES(sqlanywhere_RCC_SRCS ${sqlanywhere_RCCS})

ADD_LIBRARY (sqlanywhereplugin MODULE ${sqlanywhere_SRCS} ${sqlanywhere_MOC_SRCS} ${sqlanywhere_RCC_SRCS} ${sqlanywhere_UIS_H})

INCLUDE_DIRECTORIES(
     ${CMAKE_CURRENT_SOURCE_DIR}
     ${CMAKE_CURRENT_BINARY_DIR}
     ${CMAKE_CURRENT_BINARY_DIR}/../../ui
     ../../core
     ../../core/raster
     ../../core/renderer
     ../../core/symbology
     ../../gui
     ../../app
     ..
     ../../providers/sqlanywhere/sqlanyconnection
)

TARGET_LINK_LIBRARIES(sqlanywhereplugin
  qgis_core
  qgis_gui
  qgissqlanyconnection
)

########################################################
# Install

INSTALL(TARGETS sqlanywhereplugin
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})

