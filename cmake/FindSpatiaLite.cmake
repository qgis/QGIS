# Find SpatiaLite
# ~~~~~~~~~~~~~~~
# Copyright (c) 2009, Sandro Furieri <a.furieri at lqt.it>
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#
# CMake module to search for SpatiaLite library
#
# If it's found it sets SPATIALITE_FOUND to TRUE
# and following variables are set:
#    SPATIALITE_INCLUDE_DIR
#    SPATIALITE_LIBRARY

# This macro checks if the symbol exists
include(CheckLibraryExists)


# FIND_PATH and FIND_LIBRARY normally search standard locations
# before the specified paths. To search non-standard paths first,
# FIND_* is invoked first with specified paths and NO_DEFAULT_PATH
# and then again with no specified paths to search the default
# locations. When an earlier FIND_* succeeds, subsequent FIND_*s
# searching for the same item do nothing.

# try to use sqlite framework on mac
# want clean framework path, not unix compatibility path
IF (APPLE)
  IF (CMAKE_FIND_FRAMEWORK MATCHES "FIRST"
      OR CMAKE_FRAMEWORK_PATH MATCHES "ONLY"
      OR NOT CMAKE_FIND_FRAMEWORK)
    SET (CMAKE_FIND_FRAMEWORK_save ${CMAKE_FIND_FRAMEWORK} CACHE STRING "" FORCE)
    SET (CMAKE_FIND_FRAMEWORK "ONLY" CACHE STRING "" FORCE)
    FIND_PATH(SPATIALITE_INCLUDE_DIR SQLite3/spatialite.h)
    # if no spatialite header, we don't want sqlite find below to succeed
    IF (SPATIALITE_INCLUDE_DIR)
      FIND_LIBRARY(SPATIALITE_LIBRARY SQLite3)
      # FIND_PATH doesn't add "Headers" for a framework
      SET (SPATIALITE_INCLUDE_DIR ${SPATIALITE_LIBRARY}/Headers CACHE PATH "Path to a file." FORCE)
    ENDIF (SPATIALITE_INCLUDE_DIR)
    SET (CMAKE_FIND_FRAMEWORK ${CMAKE_FIND_FRAMEWORK_save} CACHE STRING "" FORCE)
  ENDIF ()
ENDIF (APPLE)

FIND_PATH(SPATIALITE_INCLUDE_DIR spatialite.h
  /usr/include
  "$ENV{INCLUDE}"
  "$ENV{LIB_DIR}/include"
  "$ENV{LIB_DIR}/include/spatialite"
  )

FIND_LIBRARY(SPATIALITE_LIBRARY NAMES spatialite spatialite_i PATHS
  /usr/lib
  $ENV{LIB}
  $ENV{LIB_DIR}/lib
  )

IF (SPATIALITE_INCLUDE_DIR AND SPATIALITE_LIBRARY)
   SET(SPATIALITE_FOUND TRUE)
ENDIF (SPATIALITE_INCLUDE_DIR AND SPATIALITE_LIBRARY)


IF (SPATIALITE_FOUND)

   IF (NOT SPATIALITE_FIND_QUIETLY)
      MESSAGE(STATUS "Found SpatiaLite: ${SPATIALITE_LIBRARY}")
   ENDIF (NOT SPATIALITE_FIND_QUIETLY)

   # Check for symbol gaiaDropTable
   IF(APPLE)
     # no extra LDFLAGS used in link test, may fail in OS X SDK
     SET(CMAKE_REQUIRED_LIBRARIES "-F/Library/Frameworks" ${CMAKE_REQUIRED_LIBRARIES})
   ENDIF(APPLE)
   check_library_exists("${SPATIALITE_LIBRARY}" gaiaDropTable "" SPATIALITE_VERSION_GE_4_0_0)
   check_library_exists("${SPATIALITE_LIBRARY}" gaiaStatisticsInvalidate "" SPATIALITE_VERSION_G_4_1_1)
   check_library_exists("${SPATIALITE_LIBRARY}" spatialite_init_ex "" SPATIALITE_HAS_INIT_EX)

ELSE (SPATIALITE_FOUND)

   IF (SPATIALITE_FIND_REQUIRED)
     MESSAGE(FATAL_ERROR "Could not find SpatiaLite. Include: ${SPATIALITE_INCLUDE_DIR} Library: ${SPATIALITE_LIBRARY}")
   ENDIF (SPATIALITE_FIND_REQUIRED)

ENDIF (SPATIALITE_FOUND)
