# Find DAMENG
# ~~~~~~~~~~~~~~~
# Copyright (c) 2007, Martin Dobias <wonder.sk at gmail.com>
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#
# CMake module to search for DAMENG library
#
# dm_config is searched for in DAMENG_CONFIG dir,
# default /usr/bin
#
# If it's found it sets DAMENG_FOUND to TRUE
# and following variables are set:
#    DAMENG_INCLUDE_DIR
#    DAMENG_LIBRARY

IF(ANDROID)
  SET(DAMENG_INCLUDE_DIR ${DAMENG_INCLUDE_DIR} CACHE STRING INTERNAL)
  SET(DAMENG_LIBRARY ${DM_TMP}/libdmdpi.so CACHE STRING INTERNAL)
ENDIF(ANDROID)

IF(WIN32 AND NOT ANDROID)
  IF (NOT DAMENG_INCLUDE_DIR)
    FIND_PATH(DAMENG_INCLUDE_DIR DPIext.h 
      /usr/local/include 
      /usr/include 
      c:/msys/local/include
      "$ENV{LIB_DIR}/include/dameng"
      "$ENV{LIB_DIR}/include"
      )
  ENDIF (NOT DAMENG_INCLUDE_DIR)

  IF (NOT DAMENG_LIBRARY)
    FIND_LIBRARY(DAMENG_LIBRARY NAMES dmdpi libdmdpi libdmdpidll PATHS 
      /usr/local/lib 
      /usr/lib 
      c:/msys/local/lib
      "$ENV{LIB_DIR}/lib"
      )
  ENDIF (NOT DAMENG_LIBRARY)

ELSE(WIN32)
  IF(UNIX) 

    SET(DAMENG_CONFIG_PREFER_PATH "$ENV{DAMENG_HOME}/bin" CACHE STRING "preferred path to DM (dm_config)")
    FIND_PROGRAM(DAMENG_CONFIG dm_config
      ${DAMENG_CONFIG_PREFER_PATH}
      /usr/local/dm/bin/
      /usr/local/bin/
      /usr/bin/
      )
    # MESSAGE("DBG DAMENG_CONFIG ${DAMENG_CONFIG}")
    
    IF (DAMENG_CONFIG) 
      # set INCLUDE_DIR
      EXEC_PROGRAM(${DAMENG_CONFIG}
        ARGS --includedir
        OUTPUT_VARIABLE DM_TMP)
      SET(DAMENG_INCLUDE_DIR ${DM_TMP} CACHE STRING INTERNAL)

      # set LIBRARY_DIR
      EXEC_PROGRAM(${DAMENG_CONFIG}
        ARGS --libdir
        OUTPUT_VARIABLE DM_TMP)
      IF (APPLE)
        SET(DAMENG_LIBRARY ${DM_TMP}/libdmdpi.dylib CACHE STRING INTERNAL)
      ELSEIF (CYGWIN)
        EXEC_PROGRAM(${DAMENG_CONFIG}
        ARGS --libs
        OUTPUT_VARIABLE DM_TMP)

        STRING(REGEX MATCHALL "[-][L]([^ ;])+" _LDIRS "${DM_TMP}")
        STRING(REGEX MATCHALL "[-][l]([^ ;])+" _LLIBS "${DM_TMP}")

        FIND_LIBRARY(DAMENG_LIBRARY NAMES pq PATHS /usr/lib /usr/local/lib)

        SET(_DIRS)
        FOREACH(_DIR ${_LDIRS})
          STRING(REPLACE "-L" "" _DIR ${_DIR})
          SET(_DIRS ${_DIRS} ${_DIR})
        ENDFOREACH(_DIR ${_LDIRS})

        SET(_LIBS)
        FOREACH(_LIB ${_LLIBS})
          STRING(REPLACE "-l" "" _LIB ${_LIB})
          SET(_LIBS ${_LIBS} ${_LIB})
        ENDFOREACH(_LIB ${_LDIRS})

        FOREACH(_LIBNAME ${_LIBS})
	  UNSET(DM_LIB CACHE)
          FIND_LIBRARY(DM_LIB NAMES ${_LIBNAME} PATHS ${_DIRS} /usr/lib /usr/local/lib)
	  IF(NOT DM_LIB)
	    MESSAGE(FATAL "DAMENG dependency library ${_LIBNAME} not found")
	  ENDIF(NOT DM_LIB)
          SET(DAMENG_LIBRARY ${DAMENG_LIBRARY} ${DM_LIB})
        ENDFOREACH(_LIBNAME ${_LIBS})

      ELSE (CYGWIN)
        FIND_LIBRARY(DAMENG_LIBRARY NAMES dmdpi libdmdpi libdmdpidll PATHS ${DM_TMP}/lib)
      ENDIF (APPLE)
    ENDIF(DAMENG_CONFIG)

  ENDIF(UNIX)
ENDIF(WIN32 AND NOT ANDROID)


IF (DAMENG_INCLUDE_DIR AND DAMENG_LIBRARY)
   SET(DAMENG_FOUND TRUE)
   IF(EXISTS "${DAMENG_INCLUDE_DIR}/dm_config.h")
     SET(HAVE_DMCONFIG TRUE)
   ELSE(EXISTS "${DAMENG_INCLUDE_DIR}/dm_config.h")
     SET(HAVE_DMCONFIG FALSE)
   ENDIF(EXISTS "${DAMENG_INCLUDE_DIR}/dm_config.h")
ENDIF (DAMENG_INCLUDE_DIR AND DAMENG_LIBRARY)


IF (DAMENG_FOUND)

   IF (NOT DAMENG_FIND_QUIETLY)
      MESSAGE(STATUS "Found DAMENG: ${DAMENG_LIBRARY}")
   ENDIF (NOT DAMENG_FIND_QUIETLY)

ELSE (DAMENG_FOUND)

   #SET (DAMENG_INCLUDE_DIR "")
   #SET (DAMENG_LIBRARY "")

   IF (DAMENG_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "Could not find DAMENG")
   ELSE (DAMENG_FIND_REQUIRED)
      MESSAGE(STATUS "Could not find DAMENG")
   ENDIF (DAMENG_FIND_REQUIRED)

ENDIF (DAMENG_FOUND)
