# Find RasterLite2
# ~~~~~~~~~~~~~~~
# Copyright (c) 2009, Sandro Furieri <a.furieri at lqt.it>
# Copyright (c) 2017, Mark Johnson <mj10777 at googlemail.com>
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#
# CMake module to search for RasterLite2 library
#
# If it's found it sets RASTERLITE2_FOUND to TRUE
# and following variables are set:
#    RASTERLITE2_INCLUDE_DIR
#    RASTERLITE2_LIBRARY

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
    FIND_PATH(RASTERLITE2_INCLUDE_DIR SQLite3/rasterlite2.h)
    # if no rasterlite2 header, we don't want sqlite find below to succeed
    IF (RASTERLITE2_INCLUDE_DIR)
      FIND_LIBRARY(RASTERLITE2_LIBRARY SQLite3)
      # FIND_PATH doesn't add "Headers" for a framework
      SET (RASTERLITE2_INCLUDE_DIR ${RASTERLITE2_LIBRARY}/Headers CACHE PATH "Path to a file." FORCE)
    ENDIF (RASTERLITE2_INCLUDE_DIR)
    SET (CMAKE_FIND_FRAMEWORK ${CMAKE_FIND_FRAMEWORK_save} CACHE STRING "" FORCE)
  ENDIF ()
ENDIF (APPLE)

FIND_PATH(RASTERLITE2_INCLUDE_DIR rasterlite2.h
  /usr/include
  "$ENV{INCLUDE}"
  "$ENV{LIB_DIR}/include"
  "$ENV{LIB_DIR}/include/rasterlite2"
  )

FIND_LIBRARY(RASTERLITE2_LIBRARY NAMES rasterlite2_i rasterlite2 PATHS
  /usr/lib
  $ENV{LIB}
  $ENV{LIB_DIR}/lib
  )

IF (RASTERLITE2_INCLUDE_DIR AND RASTERLITE2_LIBRARY)
   SET(RASTERLITE2_FOUND TRUE)
ENDIF (RASTERLITE2_INCLUDE_DIR AND RASTERLITE2_LIBRARY)

IF (RASTERLITE2_FOUND)

   IF (NOT RASTERLITE2_FIND_QUIETLY)
      MESSAGE(STATUS "Found RasterLite2: ${RASTERLITE2_LIBRARY}")
   ENDIF (NOT RASTERLITE2_FIND_QUIETLY)

   # Check for symbol gaiaDropTable
   IF(APPLE)
     # no extra LDFLAGS used in link test, may fail in OS X SDK
     SET(CMAKE_REQUIRED_LIBRARIES "-F/Library/Frameworks" ${CMAKE_REQUIRED_LIBRARIES})
   ENDIF(APPLE)
   # Check for symbol rl2_get_raster_map [1.1.0, 2017-04-13]
   check_library_exists("${RASTERLITE2_LIBRARY}" rl2_get_raster_map "" RASTERLITE2_VERSION_GE_1_1_0)

ELSE (RASTERLITE2_FOUND)

   IF (RASTERLITE2_FIND_REQUIRED)
     MESSAGE(FATAL_ERROR "Could not find RasterLite2. Include: ${RASTERLITE2_INCLUDE_DIR} Library: ${RASTERLITE2_LIBRARY}")
   ENDIF (RASTERLITE2_FIND_REQUIRED)

ENDIF (RASTERLITE2_FOUND)
