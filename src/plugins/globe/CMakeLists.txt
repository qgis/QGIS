# set path to additional CMake modules
SET(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules ${CMAKE_MODULE_PATH})

FIND_PACKAGE(OSG REQUIRED)
FIND_PACKAGE(OSGEARTH REQUIRED)
FIND_PACKAGE(Threads)

IF(HAVE_OSGEARTH_CHILD_SPACING)
  ADD_DEFINITIONS(-DHAVE_OSGEARTH_CHILD_SPACING)
ENDIF(HAVE_OSGEARTH_CHILD_SPACING)

########################################################
# Files

SET (globe_plugin_SRCS
     globe_plugin.cpp
     qgsosgviewer.cpp
     qgsosgearthtilesource.cpp
     globe_plugin_dialog.cpp
)

SET (globe_plugin_UIS
     globe_plugin_dialog_guibase.ui
     )

SET (globe_plugin_MOC_HDRS
     globe_plugin.h
     globe_plugin_dialog.h
     qgsosgviewer.h
)

SET (globe_plugin_RCCS  globe_plugin.qrc)

########################################################
# Build

QT4_WRAP_UI (globe_plugin_UIS_H  ${globe_plugin_UIS})

QT4_WRAP_CPP (globe_plugin_MOC_SRCS  ${globe_plugin_MOC_HDRS})

QT4_ADD_RESOURCES(globe_plugin_RCC_SRCS ${globe_plugin_RCCS})

ADD_LIBRARY (globeplugin MODULE ${globe_plugin_SRCS} ${globe_plugin_MOC_SRCS} ${globe_plugin_RCC_SRCS} ${globe_plugin_UIS_H})

INCLUDE_DIRECTORIES(
     ${CMAKE_CURRENT_BINARY_DIR}
     ${OSGEARTH_INCLUDE_DIR}
     ../../core ../../core/raster ../../core/renderer ../../core/symbology
     ../../gui
     ..
)

TARGET_LINK_LIBRARIES(globeplugin
  qgis_core
  qgis_gui
  ${QT_QTOPENGL_LIBRARY}
  ${OSGDB_LIBRARY}
  ${OSGGA_LIBRARY}
  ${OSG_LIBRARY}
  ${OSGVIEWER_LIBRARY}
  ${OSGEARTH_LIBRARY}
  ${OSGEARTHFEATURES_LIBRARY}
  ${OSGEARTHUTIL_LIBRARY}
  ${OPENTHREADS_LIBRARY}
)


########################################################
# Install

INSTALL(TARGETS globeplugin
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})

INSTALL (FILES images/world.tif
         DESTINATION ${QGIS_DATA_DIR}/globe)

INSTALL (DIRECTORY data images/gui
         DESTINATION ${QGIS_DATA_DIR}/globe)
