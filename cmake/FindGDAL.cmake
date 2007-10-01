## Once run this will define: 
## 
## GDAL_FOUND       = system has GDAL lib
##
## GDAL_LIBRARY     = full path to the library
##
## GDAL_INCLUDE_DIR      = where to find headers 
##
## Magnus Homann


IF(WIN32)

  IF (MINGW)
    FIND_PATH(GDAL_INCLUDE_DIR gdal.h /usr/local/include /usr/include c:/msys/local/include)
    FIND_LIBRARY(GDAL_LIBRARY NAMES gdal PATHS /usr/local/lib /usr/lib c:/msys/local/lib)
  ENDIF (MINGW)

  IF (MSVC)
    SET (GDAL_INCLUDE_DIR C:/dev/cpp/gdal/gcore;C:/dev/cpp/gdal/port;C:/dev/cpp/gdal/ogr;C:/dev/cpp/gdal/alg;C:/dev/cpp/gdal/ogr/ogrsf_frmts CACHE STRING INTERNAL)
    SET (GDAL_LIBRARY C:/dev/cpp/gdal/gdal.lib;odbc32;odbccp32 CACHE STRING INTERNAL)
  ENDIF (MSVC)
  
  
ELSE(WIN32)
  IF(UNIX) 

    # try to use framework on mac
    IF (APPLE)
      SET (GDAL_MAC_PATH /Library/Frameworks/GDAL.framework/unix/bin)
    ENDIF (APPLE)

    SET(GDAL_CONFIG_PREFER_PATH "$ENV{GDAL_HOME}/bin" CACHE STRING "preferred path to GDAL (gdal-config)")
    FIND_PROGRAM(GDAL_CONFIG gdal-config
      ${GDAL_CONFIG_PREFER_PATH}
      ${GDAL_MAC_PATH}
      /usr/local/bin/
      /usr/bin/
      )
    # MESSAGE("DBG GDAL_CONFIG ${GDAL_CONFIG}")
    
    IF (GDAL_CONFIG) 
      # set INCLUDE_DIR to prefix+include
      EXEC_PROGRAM(${GDAL_CONFIG}
        ARGS --prefix
        OUTPUT_VARIABLE GDAL_PREFIX)
      #SET(GDAL_INCLUDE_DIR ${GDAL_PREFIX}/include CACHE STRING INTERNAL)
      FIND_PATH(GDAL_INCLUDE_DIR 
        gdal.h 
        ${GDAL_PREFIX}/include/gdal
        ${GDAL_PREFIX}/include
        /usr/local/include 
        /usr/include 
        )

      ## extract link dirs for rpath  
      EXEC_PROGRAM(${GDAL_CONFIG}
        ARGS --libs
        OUTPUT_VARIABLE GDAL_CONFIG_LIBS )

      ## split off the link dirs (for rpath)
      ## use regular expression to match wildcard equivalent "-L*<endchar>"
      ## with <endchar> is a space or a semicolon
      STRING(REGEX MATCHALL "[-][L]([^ ;])+" 
        GDAL_LINK_DIRECTORIES_WITH_PREFIX 
        "${GDAL_CONFIG_LIBS}" )
        #      MESSAGE("DBG  GDAL_LINK_DIRECTORIES_WITH_PREFIX=${GDAL_LINK_DIRECTORIES_WITH_PREFIX}")

      ## remove prefix -L because we need the pure directory for LINK_DIRECTORIES
      
      IF (GDAL_LINK_DIRECTORIES_WITH_PREFIX)
        STRING(REGEX REPLACE "[-][L]" "" GDAL_LINK_DIRECTORIES ${GDAL_LINK_DIRECTORIES_WITH_PREFIX} )
      ENDIF (GDAL_LINK_DIRECTORIES_WITH_PREFIX)

      ## split off the name
      ## use regular expression to match wildcard equivalent "-l*<endchar>"
      ## with <endchar> is a space or a semicolon
      STRING(REGEX MATCHALL "[-][l]([^ ;])+" 
        GDAL_LIB_NAME_WITH_PREFIX 
        "${GDAL_CONFIG_LIBS}" )
        #      MESSAGE("DBG  GDAL_LIB_NAME_WITH_PREFIX=${GDAL_LIB_NAME_WITH_PREFIX}")


      ## remove prefix -l because we need the pure name
      
      IF (GDAL_LIB_NAME_WITH_PREFIX)
        STRING(REGEX REPLACE "[-][l]" "" GDAL_LIB_NAME ${GDAL_LIB_NAME_WITH_PREFIX} )
      ENDIF (GDAL_LIB_NAME_WITH_PREFIX)

      IF (APPLE)
        SET(GDAL_LIBRARY ${GDAL_LINK_DIRECTORIES}/lib${GDAL_LIB_NAME}.dylib CACHE STRING INTERNAL)
      ELSE (APPLE)
       SET(GDAL_LIBRARY ${GDAL_LINK_DIRECTORIES}/lib${GDAL_LIB_NAME}.so CACHE STRING INTERNAL)
      ENDIF (APPLE)
      
    ELSE(GDAL_CONFIG)
      MESSAGE("FindGDAL.cmake: gdal-config not found. Please set it manually. GDAL_CONFIG=${GDAL_CONFIG}")
    ENDIF(GDAL_CONFIG)

  ENDIF(UNIX)
ENDIF(WIN32)


IF (GDAL_INCLUDE_DIR AND GDAL_LIBRARY)
   SET(GDAL_FOUND TRUE)
ENDIF (GDAL_INCLUDE_DIR AND GDAL_LIBRARY)

IF (GDAL_FOUND)

   IF (NOT GDAL_FIND_QUIETLY)
      MESSAGE(STATUS "Found GDAL: ${GDAL_LIBRARY}")
   ENDIF (NOT GDAL_FIND_QUIETLY)

ELSE (GDAL_FOUND)

   MESSAGE(GDAL_INCLUDE_DIR=${GDAL_INCLUDE_DIR})
   MESSAGE(GDAL_LIBRARY=${GDAL_LIBRARY})
   MESSAGE(FATAL_ERROR "Could not find GDAL")

ENDIF (GDAL_FOUND)
