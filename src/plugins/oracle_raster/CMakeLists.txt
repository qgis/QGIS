
########################################################
# Files

SET (ORACLE_SRCS
     qgsoracle_plugin.cpp
     qgsselectgeoraster_ui.cpp
     qgsoracleconnect_ui.cpp
)

SET (ORACLE_UIS
     qgsselectgeorasterbase.ui
     qgsoracleconnectbase.ui
)

SET (ORACLE_MOC_HDRS
     qgsoracle_plugin.h
     qgsselectgeoraster_ui.h
     qgsoracleconnect_ui.h
)

SET (ORACLE_RCCS  oracleplugin.qrc)

########################################################
# Build

QT4_WRAP_UI (ORACLE_UIS_H  ${ORACLE_UIS})

QT4_WRAP_CPP (ORACLE_MOC_SRCS  ${ORACLE_MOC_HDRS})

QT4_ADD_RESOURCES(ORACLE_RCC_SRCS ${ORACLE_RCCS})

ADD_LIBRARY (oracleplugin MODULE ${ORACLE_SRCS} ${ORACLE_MOC_SRCS} ${ORACLE_RCC_SRCS} ${ORACLE_UIS_H})

INCLUDE_DIRECTORIES(
     ${CMAKE_CURRENT_BINARY_DIR}
     ../../core ../../core/raster
     ../../gui
     ..
     ${GDAL_INCLUDE_DIR}
)

TARGET_LINK_LIBRARIES(oracleplugin
  qgis_core
  qgis_gui
)


########################################################
# Install

INSTALL(TARGETS oracleplugin
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})

