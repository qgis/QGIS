# Find PostgreSQL
# ~~~~~~~~~~~~~~~
# Copyright (c) 2007, Martin Dobias <wonder.sk at gmail.com>
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#
# CMake module to search for PostgreSQL library
#
# pg_config is searched for in POSTGRES_CONFIG dir,
# default /usr/bin
#
# If it's found it sets POSTGRES_FOUND to TRUE
# and following variables are set:
#    POSTGRES_INCLUDE_DIR
#    POSTGRES_LIBRARY

IF(ANDROID)
  SET(POSTGRES_INCLUDE_DIR ${POSTGRES_INCLUDE_DIR} CACHE STRING INTERNAL)
  SET(POSTGRES_LIBRARY ${PG_TMP}/libpq.so CACHE STRING INTERNAL)
ENDIF(ANDROID)

IF(WIN32 AND NOT ANDROID)
  IF (NOT POSTGRES_INCLUDE_DIR)
    FIND_PATH(POSTGRES_INCLUDE_DIR libpq-fe.h 
      /usr/local/include 
      /usr/include 
      c:/msys/local/include
      "$ENV{LIB_DIR}/include/postgresql"
      "$ENV{LIB_DIR}/include"
      )
  ENDIF (NOT POSTGRES_INCLUDE_DIR)

  IF (NOT POSTGRES_LIBRARY)
    FIND_LIBRARY(POSTGRES_LIBRARY NAMES pq libpq libpqdll PATHS 
      /usr/local/lib 
      /usr/lib 
      c:/msys/local/lib
      "$ENV{LIB_DIR}/lib"
      )
  ENDIF (NOT POSTGRES_LIBRARY)

ELSE(WIN32)
  IF(UNIX) 

    SET(POSTGRES_CONFIG_PREFER_PATH "$ENV{POSTGRES_HOME}/bin" CACHE STRING "preferred path to PG (pg_config)")
    FIND_PROGRAM(POSTGRES_CONFIG pg_config
      ${POSTGRES_CONFIG_PREFER_PATH}
      /usr/local/pgsql/bin/
      /usr/local/bin/
      /usr/bin/
      )
    # MESSAGE("DBG POSTGRES_CONFIG ${POSTGRES_CONFIG}")
    
    IF (POSTGRES_CONFIG) 
      # set INCLUDE_DIR
      EXEC_PROGRAM(${POSTGRES_CONFIG}
        ARGS --includedir
        OUTPUT_VARIABLE PG_TMP)
      SET(POSTGRES_INCLUDE_DIR ${PG_TMP} CACHE STRING INTERNAL)

      # set LIBRARY_DIR
      EXEC_PROGRAM(${POSTGRES_CONFIG}
        ARGS --libdir
        OUTPUT_VARIABLE PG_TMP)
      IF (APPLE)
        SET(POSTGRES_LIBRARY ${PG_TMP}/libpq.dylib CACHE STRING INTERNAL)
      ELSEIF (CYGWIN)
        EXEC_PROGRAM(${POSTGRES_CONFIG}
        ARGS --libs
        OUTPUT_VARIABLE PG_TMP)

        STRING(REGEX MATCHALL "[-][L]([^ ;])+" _LDIRS "${PG_TMP}")
        STRING(REGEX MATCHALL "[-][l]([^ ;])+" _LLIBS "${PG_TMP}")

        FIND_LIBRARY(POSTGRES_LIBRARY NAMES pq PATHS /usr/lib /usr/local/lib)

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
	  UNSET(PG_LIB CACHE)
          FIND_LIBRARY(PG_LIB NAMES ${_LIBNAME} PATHS ${_DIRS} /usr/lib /usr/local/lib)
	  IF(NOT PG_LIB)
	    MESSAGE(FATAL "PostgreSQL dependency library ${_LIBNAME} not found")
	  ENDIF(NOT PG_LIB)
          SET(POSTGRES_LIBRARY ${POSTGRES_LIBRARY} ${PG_LIB})
        ENDFOREACH(_LIBNAME ${_LIBS})

      ELSE (CYGWIN)
        SET(POSTGRES_LIBRARY ${PG_TMP}/libpq.so CACHE STRING INTERNAL)
      ENDIF (APPLE)
    ENDIF(POSTGRES_CONFIG)

  ENDIF(UNIX)
ENDIF(WIN32 AND NOT ANDROID)


IF (POSTGRES_INCLUDE_DIR AND POSTGRES_LIBRARY)
   SET(POSTGRES_FOUND TRUE)
   IF(EXISTS "${POSTGRES_INCLUDE_DIR}/pg_config.h")
     SET(HAVE_PGCONFIG TRUE)
   ELSE(EXISTS "${POSTGRES_INCLUDE_DIR}/pg_config.h")
     SET(HAVE_PGCONFIG FALSE)
   ENDIF(EXISTS "${POSTGRES_INCLUDE_DIR}/pg_config.h")
ENDIF (POSTGRES_INCLUDE_DIR AND POSTGRES_LIBRARY)


IF (POSTGRES_FOUND)

   IF (NOT POSTGRES_FIND_QUIETLY)
      MESSAGE(STATUS "Found PostgreSQL: ${POSTGRES_LIBRARY}")
   ENDIF (NOT POSTGRES_FIND_QUIETLY)

ELSE (POSTGRES_FOUND)

   #SET (POSTGRES_INCLUDE_DIR "")
   #SET (POSTGRES_LIBRARY "")

   IF (POSTGRES_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "Could not find PostgreSQL")
   ELSE (POSTGRES_FIND_REQUIRED)
      MESSAGE(STATUS "Could not find PostgreSQL")
   ENDIF (POSTGRES_FIND_REQUIRED)

ENDIF (POSTGRES_FOUND)
