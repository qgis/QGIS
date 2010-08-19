
# set path to additional CMake modules
SET(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules ${CMAKE_MODULE_PATH})

FIND_PACKAGE(OSG REQUIRED)
FIND_PACKAGE(OSGEARTH REQUIRED)
FIND_PACKAGE(Threads)

########################################################
# Files

SET (globe_plugin_SRCS
     globe_plugin.cpp
     globe_plugin_gui.cpp
     qgsosgviewer.cpp
     qgsosgearthtilesource.cpp
)

SET (globe_plugin_UIS globe_plugin_guibase.ui)

SET (globe_plugin_MOC_HDRS
     globe_plugin.h
     globe_plugin_gui.h
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
     ../../core ../../core/raster ../../core/renderer ../../core/symbology
     ../../gui
     ..
)

# SET(TARGET_LIBRARIES_VARS OSG_LIBRARY OSGDB_LIBRARY OSGUTIL_LIBRARY OSGVIEWER_LIBRARY OPENTHREADS_LIBRARY)

TARGET_LINK_LIBRARIES(globeplugin
  qgis_core
  qgis_gui
  QtOpenGL
  osgDB
  osgGA
  osg
  osgViewer
  osgEarth
  osgEarthFeatures
  osgEarthUtil
  ${OPENTHREADS_LIBRARY}
)


########################################################
# Install

INSTALL(TARGETS globeplugin
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})
