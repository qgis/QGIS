## Once run this will define: 
## 
## QGIS_FOUND       = system has QGIS lib
##
## QGIS_CORE_LIBRARY     = full path to the CORE library
## QGIS_GUI_LIBRARY      = full path to the GUI library
## QGIS_ANALYSIS_LIBRARY = full path to the ANALYSIS library
## QGIS_PLUGIN_DIR       = full path to where QGIS plugins are installed
## QGIS_INCLUDE_DIR      = where to find headers 
##
## Tim Sutton

#MESSAGE("Searching for QGIS")
IF(WIN32)
  #MESSAGE("Searching for QGIS in $ENV{PROGRAMFILES}/Quantum GIS")
  IF (MINGW)
    FIND_PATH(QGIS_PLUGIN_DIR
      NAMES libnortharrowplugin.dll
      PATHS
        "$ENV{PROGRAMFILES}/Quantum GIS/plugins"
    )
    FIND_PATH(QGIS_INCLUDE_DIR
      NAMES qgsapplication.h 
      PATHS
        "$ENV{PROGRAMFILES}/Quantum GIS/include" 
    )
    FIND_LIBRARY(QGIS_CORE_LIBRARY
      NAMES qgis_core
      PATHS
        "$ENV{PROGRAMFILES}/Quantum GIS/"
    )
    FIND_LIBRARY(QGIS_GUI_LIBRARY
      NAMES qgis_gui
      PATHS
        "$ENV{PROGRAMFILES}/Quantum GIS/"
    )
  ENDIF (MINGW)

  IF (MSVC)
    FIND_PATH(QGIS_PLUGIN_DIR
      NAMES northarrowplugin.dll
      PATHS
        "$ENV{OSGEO4W_ROOT}/apps/qgis/plugins"
        "$ENV{PROGRAMFILES}/Quantum GIS/plugins"
    )
    FIND_PATH(QGIS_INCLUDE_DIR 
      NAMES qgsapplication.h 
      PATHS
        "$ENV{INCLUDE}" 
        "$ENV{LIB_DIR}/include/qgis" 
        "$ENV{OSGEO4W_ROOT}/include"
        "$ENV{PROGRAMFILES}/Quantum GIS/include"
    )
    FIND_LIBRARY(QGIS_CORE_LIBRARY
      NAMES qgis_core
      PATHS 
        "$ENV{LIB_DIR}/lib/" 
        "$ENV{LIB}" 
        "$ENV{OSGEO4W_ROOT}/lib"
        "$ENV{PROGRAMFILES}/Quantum GIS/lib"
    )
    FIND_LIBRARY(QGIS_GUI_LIBRARY
      NAMES qgis_gui
      PATHS 
        "$ENV{LIB_DIR}" 
        "$ENV{LIB}" 
        "$ENV{OSGEO4W_ROOT}/lib"
        "$ENV{PROGRAMFILES}/Quantum GIS/lib"
    )
    FIND_LIBRARY(QGIS_ANALYSIS_LIBRARY
      NAMES qgis_analysis
      PATHS 
        "$ENV{LIB_DIR}" 
        "$ENV{LIB}" 
        "$ENV{OSGEO4W_ROOT}/lib"
        "$ENV{PROGRAMFILES}/Quantum GIS/lib"
    )
  ENDIF (MSVC)
ELSE(WIN32)
  IF(UNIX) 
    # try to use bundle on mac
    SET (QGIS_MAC_PATH /Applications/QGIS.app/Contents)
    #MESSAGE("Searching for QGIS in /usr/bin; /usr/local/bin")
    FIND_PATH(QGIS_PLUGIN_DIR
      NAMES libnortharrowplugin.so
      PATHS
        /usr/lib64/qgis/plugins
        /usr/lib/qgis
        /usr/local/lib/qgis/plugins
        ${QGIS_MAC_PATH}/PlugIns/qgis
        "$ENV{LIB_DIR}/lib/qgis/plugins" 
        "$ENV{LIB_DIR}/lib/qgis" 
    )
    FIND_PATH(QGIS_INCLUDE_DIR
      NAMES qgis.h 
      PATHS
        /usr/include/qgis
        /usr/local/include/qgis
        ${QGIS_MAC_PATH}/Frameworks/qgis_core.framework/Headers
        "$ENV{LIB_DIR}/include/qgis" 
    )
    # also get other frameworks' headers folders on OS X
    IF (APPLE)
      FIND_PATH(QGIS_GUI_INCLUDE_DIR
        NAMES qgisgui.h 
        PATHS ${QGIS_MAC_PATH}/Frameworks/qgis_gui.framework/Headers
      )
      FIND_PATH(QGIS_ANALYSIS_INCLUDE_DIR
        NAMES qgsinterpolator.h 
        PATHS ${QGIS_MAC_PATH}/Frameworks/qgis_analysis.framework/Headers
      )
      SET(QGIS_INCLUDE_DIR
        ${QGIS_INCLUDE_DIR}
        ${QGIS_GUI_INCLUDE_DIR}
        ${QGIS_ANALYSIS_INCLUDE_DIR}
      )
    ENDIF (APPLE)
    FIND_LIBRARY(QGIS_CORE_LIBRARY
      NAMES qgis_core
      PATHS 
        /usr/lib64
        /usr/lib
        /usr/local/lib
        ${QGIS_MAC_PATH}/Frameworks
        "$ENV{LIB_DIR}/lib/" 
    )
    FIND_LIBRARY(QGIS_GUI_LIBRARY
      NAMES qgis_gui
      PATHS 
        /usr/lib64
        /usr/lib
        /usr/local/lib
        ${QGIS_MAC_PATH}/Frameworks
        "$ENV{LIB_DIR}/lib/" 
    )
    FIND_LIBRARY(QGIS_ANALYSIS_LIBRARY
      NAMES qgis_analysis
      PATHS 
        /usr/lib64
        /usr/lib
        /usr/local/lib
        ${QGIS_MAC_PATH}/Frameworks
        "$ENV{LIB_DIR}/lib/" 
    )
  ENDIF(UNIX)
ENDIF(WIN32)

IF (QGIS_INCLUDE_DIR AND QGIS_CORE_LIBRARY AND QGIS_GUI_LIBRARY AND QGIS_ANALYSIS_LIBRARY)
   SET(QGIS_FOUND TRUE)
ENDIF ()

IF (QGIS_FOUND)
   IF (NOT QGIS_FIND_QUIETLY)
     MESSAGE(STATUS "Found QGIS core: ${QGIS_CORE_LIBRARY}")
     MESSAGE(STATUS "Found QGIS gui: ${QGIS_GUI_LIBRARY}")
     MESSAGE(STATUS "Found QGIS analysis: ${QGIS_ANALYSIS_LIBRARY}")
     MESSAGE(STATUS "Found QGIS plugins directory: ${QGIS_PLUGIN_DIR}")
   ENDIF (NOT QGIS_FIND_QUIETLY)
ELSE (QGIS_FOUND)
   IF (QGIS_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "Could not find QGIS")
   ENDIF (QGIS_FIND_REQUIRED)
ENDIF (QGIS_FOUND)
