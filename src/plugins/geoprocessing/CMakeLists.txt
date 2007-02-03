
########################################################
# Files

SET (GEOPROC_SRCS
     qgspggeoprocessing.cpp
     qgsdlgpgbuffer.cpp
)

SET (GEOPROC_UIS qgsdlgpgbufferbase.ui)

SET (GEOPROC_MOC_HDRS
     qgspggeoprocessing.h
     qgsdlgpgbuffer.h
)

########################################################
# Build

QT4_WRAP_UI (GEOPROC_UIS_H  ${GEOPROC_UIS})

QT4_WRAP_CPP (GEOPROC_MOC_SRCS  ${GEOPROC_MOC_HDRS})

ADD_LIBRARY (pggeoprocessingplugin MODULE ${GEOPROC_SRCS} ${GEOPROC_MOC_SRCS} ${GEOPROC_UIS_H})

INCLUDE_DIRECTORIES(
     ${CMAKE_CURRENT_BINARY_DIR}
     ../../core ../../raster ../../renderer ../../core/symbology
     ../../gui
     ${CMAKE_CURRENT_BINARY_DIR}/../../ui
     ..
     ${POSTGRES_INCLUDE_DIR}
)

TARGET_LINK_LIBRARIES(pggeoprocessingplugin
  ${POSTGRES_LIBRARY}
  qgis_core
)


########################################################
# Install

INSTALL(TARGETS pggeoprocessingplugin
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})
