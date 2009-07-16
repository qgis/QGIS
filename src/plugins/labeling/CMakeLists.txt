
########################################################
# Files

SET (labeling_SRCS
     labeling.cpp
     labelinggui.cpp
     pallabeling.cpp
     engineconfigdialog.cpp
     labelpreview.cpp
)

SET (labeling_UIS labelingguibase.ui engineconfigdialog.ui)

SET (labeling_MOC_HDRS
     labeling.h
     labelinggui.h
     engineconfigdialog.h
)

SET (labeling_RCCS  labeling.qrc)

########################################################
# Build

QT4_WRAP_UI (labeling_UIS_H  ${labeling_UIS})

QT4_WRAP_CPP (labeling_MOC_SRCS  ${labeling_MOC_HDRS})

QT4_ADD_RESOURCES(labeling_RCC_SRCS ${labeling_RCCS})

ADD_LIBRARY (labelingplugin MODULE ${labeling_SRCS} ${labeling_MOC_SRCS} ${labeling_RCC_SRCS} ${labeling_UIS_H})

INCLUDE_DIRECTORIES(
     ${CMAKE_CURRENT_BINARY_DIR}
     ../../core ../../core/pal ../../core/raster ../../core/renderer ../../core/symbology
     ../../gui
     ..
     .
     ${GEOS_INCLUDE_DIR}
)

TARGET_LINK_LIBRARIES(labelingplugin
  qgis_core
  qgis_gui
)


########################################################
# Install

INSTALL(TARGETS labelingplugin
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})

