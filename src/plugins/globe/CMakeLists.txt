# set path to additional CMake modules
SET(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules ${CMAKE_MODULE_PATH})

FIND_PACKAGE(OSG REQUIRED)
FIND_PACKAGE(OSGEARTH REQUIRED)
FIND_PACKAGE(Threads)

########################################################
# Files

SET (GLOBE_PLUGIN_SRCS
    globe_plugin.cpp
    qgsglobetilesource.cpp
    qgsglobeplugindialog.cpp
    qgsglobevectorlayerproperties.cpp
    qgsglobefeatureidentify.cpp
    qgsglobefrustumhighlight.cpp
    qgsglobewidget.cpp
)

SET (GLOBE_PLUGIN_UIS
    qgsglobeplugindialog.ui
    qgsglobevectorlayerpropertiespage.ui
)

SET (GLOBE_PLUGIN_MOC_HDRS
    globe_plugin.h
    qgsglobeplugindialog.h
    qgsglobevectorlayerproperties.h
    qgsglobetilesource.h
    qgsglobewidget.h
)

SET (GLOBE_PLUGIN_HDRS
    qgsglobevectorlayerproperties.h
    qgsglobefeatureidentify.h
    qgsglobefrustumhighlight.h
)

SET (GLOBE_PLUGIN_RCCS  globe_plugin.qrc)

########################################################
# Build

IF(WIN32)
    ADD_DEFINITIONS("\"-DGLOBE_EXPORT=${DLLEXPORT}\"")
ELSE(WIN32)
    ADD_DEFINITIONS("-DGLOBE_EXPORT=")
ENDIF(WIN32)

QT4_WRAP_UI (GLOBE_PLUGIN_UIS_H  ${GLOBE_PLUGIN_UIS})

QT4_WRAP_CPP (GLOBE_PLUGIN_MOC_SRCS  ${GLOBE_PLUGIN_MOC_HDRS})

QT4_ADD_RESOURCES(GLOBE_PLUGIN_RCC_SRCS ${GLOBE_PLUGIN_RCCS})

ADD_LIBRARY (globeplugin SHARED ${GLOBE_PLUGIN_SRCS} ${GLOBE_PLUGIN_MOC_SRCS} ${GLOBE_PLUGIN_RCC_SRCS} ${GLOBE_PLUGIN_UIS_H} ${GLOBE_PLUGIN_HDRS})

INCLUDE_DIRECTORIES(
     ${CMAKE_CURRENT_BINARY_DIR}
     ${OSGEARTH_INCLUDE_DIR}
     ${OSG_INCLUDE_DIR}
     ${GEOS_INCLUDE_DIR}
     ${SIP_INCLUDE_DIR}
     ../../core
     ../../core/geometry
     ../../core/raster
     ../../core/symbology-ng
     ../../gui
     ..
)

TARGET_LINK_LIBRARIES(globeplugin
  qgis_core
  qgis_gui
  ${QT_QTOPENGL_LIBRARY}
  ${OSGDB_LIBRARY}
  ${OSGGA_LIBRARY}
  ${OSGUTIL_LIBRARY}
  ${OSG_LIBRARY}
  ${OSGQT_LIBRARY}
  ${OSGVIEWER_LIBRARY}
  ${OSGEARTH_LIBRARY}
  ${OSGEARTHANNOTATION_LIBRARY}
  ${OSGEARTHFEATURES_LIBRARY}
  ${OSGEARTHUTIL_LIBRARY}
  ${OSGEARTHSYMBOLOGY_LIBRARY}
  ${OSGEARTHQT_LIBRARY}
  ${OPENTHREADS_LIBRARY}
)

ADD_SUBDIRECTORY(featuresource)

########################################################
# Install

INSTALL(TARGETS globeplugin
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})

INSTALL (FILES images/world.tif
         DESTINATION ${QGIS_DATA_DIR}/globe)

INSTALL (DIRECTORY images/gui
         DESTINATION ${QGIS_DATA_DIR}/globe)
