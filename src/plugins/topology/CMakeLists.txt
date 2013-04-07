
########################################################
# Files

SET (topol_SRCS
  topol.cpp
  rulesDialog.cpp
  checkDock.cpp
  topolError.cpp
  topolTest.cpp
  dockModel.cpp
)

SET (topol_UIS
  rulesDialog.ui
  checkDock.ui  
)

SET (topol_MOC_HDRS
  topol.h
  rulesDialog.h
  checkDock.h
  topolTest.h
  dockModel.h
)

SET (topol_RCCS  topol.qrc)

########################################################
# Build

QT4_WRAP_UI (topol_UIS_H  ${topol_UIS})

QT4_WRAP_CPP (topol_MOC_SRCS  ${topol_MOC_HDRS})

QT4_ADD_RESOURCES(topol_RCC_SRCS ${topol_RCCS})

ADD_LIBRARY (topolplugin MODULE ${topol_SRCS} ${topol_MOC_SRCS} ${topol_RCC_SRCS} ${topol_UIS_H})

INCLUDE_DIRECTORIES(
  ${CMAKE_BINARY_DIR}/src/ui
  ${CMAKE_CURRENT_BINARY_DIR}
  ${GEOS_INCLUDE_DIR}
  ../../core ../../core/raster
  ../../gui  
  ..
)

TARGET_LINK_LIBRARIES(topolplugin
  qgis_core
  qgis_gui
)


########################################################
# Install

INSTALL(TARGETS topolplugin
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})

