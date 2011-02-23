## Once run this will define: 
## 
## QGIS_FOUND       = system has QGIS lib
##
## QGIS_CORE_LIBRARY     = full path to the CORE library
## QGIS_GUI_LIBRARY      = full path to the GUI library
## QGIS_PLUGIN_DIR       = full path to where QGIS plugins are installed
## QGIS_INCLUDE_DIR      = where to find headers 
##
## Tim Sutton

#MESSAGE("Searching for QGIS")
IF(WIN32)
  #MESSAGE("Searching for QGIS in C:/program files/Quantum GIS")
  IF (MINGW)
  FIND_PATH(QGIS_PLUGIN_DIR libnortharrowplugin.dll 
	  "C:/Program Files/Quantum GIS/plugins" 
	  )
  FIND_PATH(QGIS_INCLUDE_DIR qgsapplication.h 
	  "C:/Program Files/Quantum GIS/include" 
	  )
  FIND_LIBRARY(QGIS_CORE_LIBRARY NAMES qgis_core PATHS 
	  "C:/Program Files/Quantum GIS/" 
	  )
  FIND_LIBRARY(QGIS_GUI_LIBRARY NAMES qgis_gui PATHS 
	  "C:/Program Files/Quantum GIS/" 
	  )
  ENDIF (MINGW)

  IF (MSVC)
  FIND_PATH(QGIS_PLUGIN_DIR libnortharrowplugin.dll 
    "C:/OSGeo4W/app/qgis/plugins"
	  "C:/Program Files/Quantum GIS/lib/qgis" 
	  )
  FIND_PATH(QGIS_INCLUDE_DIR qgsapplication.h 
    "C:/OSGeo4W/include"
	  "$ENV{LIB_DIR}/include/qgis" 
	  )
  FIND_LIBRARY(QGIS_CORE_LIBRARY NAMES qgis_core PATHS 
    "C:/OSGeo4W/lib"
	  "$ENV{LIB_DIR}/lib/" 
	  )
  FIND_LIBRARY(QGIS_GUI_LIBRARY NAMES qgis_gui PATHS 
    "C:/OSGeo4W/lib"
	  "$ENV{LIB_DIR}/lib/" 
	  )
  ENDIF (MSVC)

ELSE(WIN32)
  IF(UNIX) 

    # try to use bundle on mac
    IF (APPLE)
      #MESSAGE("Searching for QGIS in /Applications/QGIS.app/Contents/MacOS")
      #SET (QGIS_MAC_PATH /Applications/qgis1.0.0.app/Contents/MacOS)
      SET (QGIS_MAC_PATH /Applications/qgis1.1.0.app/Contents/MacOS)
      SET (QGIS_LIB_DIR ${QGIS_MAC_PATH}/lib)
      SET (QGIS_PLUGIN_DIR ${QGIS_MAC_PATH}/lib/qgis CACHE STRING INTERNAL)
      # set INCLUDE_DIR to prefix+include
      SET(QGIS_INCLUDE_DIR ${QGIS_MAC_PATH}/include/qgis CACHE STRING INTERNAL)
      ## extract link dirs 
      SET(QGIS_CORE_LIBRARY ${QGIS_LIB_DIR}/libqgis_core.dylib CACHE STRING INTERNAL)
      SET(QGIS_GUI_LIBRARY ${QGIS_LIB_DIR}/libqgis_gui.dylib CACHE STRING INTERNAL)
    ELSE (APPLE)
      #MESSAGE("Searching for QGIS in /usr/bin; /usr/local/bin")
      FIND_PATH(QGIS_PLUGIN_DIR libnortharrowplugin.so
        /usr/lib64/qgis/plugins
      	/usr/lib/qgis
        /usr/local/lib/qgis/plugins
	      "$ENV{LIB_DIR}/lib/qgis/plugins" 
	      "$ENV{LIB_DIR}/lib/qgis" 
      )
      FIND_PATH(QGIS_INCLUDE_DIR qgis.h 
       /usr/include/qgis
       /usr/local/include/qgis
	     "$ENV{LIB_DIR}/include/qgis" 
       )
      FIND_LIBRARY(QGIS_CORE_LIBRARY NAMES qgis_core PATHS 
       /usr/lib64
       /usr/lib
       /usr/local/lib
	     "$ENV{LIB_DIR}/lib/" 
       )
       FIND_LIBRARY(QGIS_GUI_LIBRARY NAMES qgis_gui PATHS 
       /usr/lib64
       /usr/lib
       /usr/local/lib
	     "$ENV{LIB_DIR}/lib/" 
       )
    ENDIF (APPLE)
  ENDIF(UNIX)
ENDIF(WIN32)


IF (QGIS_INCLUDE_DIR AND QGIS_CORE_LIBRARY AND QGIS_GUI_LIBRARY)
   SET(QGIS_FOUND TRUE)
ENDIF (QGIS_INCLUDE_DIR AND QGIS_CORE_LIBRARY AND QGIS_GUI_LIBRARY)

IF (QGIS_FOUND)
   IF (NOT QGIS_FIND_QUIETLY)
     MESSAGE(STATUS "Found QGIS Core: ${QGIS_CORE_LIBRARY}")
     MESSAGE(STATUS "Found QGIS Gui: ${QGIS_GUI_LIBRARY}")
     MESSAGE(STATUS "Found QGIS Plugins Dir: ${QGIS_PLUGIN_DIR}")
   ENDIF (NOT QGIS_FIND_QUIETLY)
ELSE (QGIS_FOUND)
   IF (QGIS_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "Could not find QGIS")
   ENDIF (QGIS_FIND_REQUIRED)
ENDIF (QGIS_FOUND)
