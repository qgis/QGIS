# Find PDAL
# ~~~~~~~~~~
# Copyright (c) 2020, Peter Petrik <zilolv at gmail.com>
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#
# CMake module to search for PDAL library
#
# If it's found it sets PDAL_FOUND to TRUE
# and following variables are set:
#    PDAL_INCLUDE_DIR
#    PDAL_LIBRARIES

# FIND_PATH and FIND_LIBRARY normally search standard locations
# before the specified paths. To search non-standard paths first,
# FIND_* is invoked first with specified paths and NO_DEFAULT_PATH
# and then again with no specified paths to search the default
# locations. When an earlier FIND_* succeeds, subsequent FIND_*s
# searching for the same item do nothing.

FIND_PATH(PDAL_INCLUDE_DIR pdal/pdal.hpp
  "$ENV{LIB_DIR}/include"
  "/usr/include"
  c:/msys/local/include
  NO_DEFAULT_PATH
  )
FIND_PATH(PDAL_INCLUDE_DIR pdal/pdal.hpp)

FIND_LIBRARY(PDAL_CPP_LIBRARY NAMES pdalcpp libpdalcpp PATHS
  "$ENV{LIB_DIR}/lib"
  c:/msys/local/lib
  NO_DEFAULT_PATH
  )
FIND_LIBRARY(PDAL_CPP_LIBRARY NAMES pdalcpp libpdalcpp)

FIND_LIBRARY(PDAL_UTIL_LIBRARY NAMES pdal_util libpdal_util PATHS
  "$ENV{LIB_DIR}/lib"
  c:/msys/local/lib
  NO_DEFAULT_PATH
  )
FIND_LIBRARY(PDAL_UTIL_LIBRARY NAMES pdal_util libpdal_util)

FIND_PROGRAM(PDAL_BIN pdal
    $ENV{LIB_DIR}/bin
    /usr/local/bin/
    /usr/bin/
    NO_DEFAULT_PATH
    )
FIND_PROGRAM(PDAL_BIN pdal)

IF (PDAL_INCLUDE_DIR AND PDAL_CPP_LIBRARY AND PDAL_UTIL_LIBRARY AND PDAL_BIN)
   SET(PDAL_FOUND TRUE)
   SET(PDAL_LIBRARIES ${PDAL_CPP_LIBRARY} ${PDAL_UTIL_LIBRARY})
ENDIF (PDAL_INCLUDE_DIR AND PDAL_CPP_LIBRARY AND PDAL_UTIL_LIBRARY AND PDAL_BIN)

IF (PDAL_FOUND)
    # extract PDAL version
    EXEC_PROGRAM(${PDAL_BIN}
	ARGS --version
	OUTPUT_VARIABLE PDAL_VERSION_OUT )
    STRING(REGEX REPLACE "^.*([0-9]+)\\.([0-9]+)\\.([0-9]+).*$" "\\1" PDAL_VERSION_MAJOR "${PDAL_VERSION_OUT}")
    STRING(REGEX REPLACE "^.*([0-9]+)\\.([0-9]+)\\.([0-9]+).*$" "\\2" PDAL_VERSION_MINOR "${PDAL_VERSION_OUT}")
    STRING(REGEX REPLACE "^.*([0-9]+)\\.([0-9]+)\\.([0-9]+).*$" "\\3" PDAL_VERSION_MICRO "${PDAL_VERSION_OUT}")
    STRING(CONCAT PDAL_VERSION ${PDAL_VERSION_MAJOR} "." ${PDAL_VERSION_MINOR} "." ${PDAL_VERSION_MICRO})

   IF (NOT PDAL_FIND_QUIETLY)
      MESSAGE(STATUS "Found PDAL: ${PDAL_LIBRARIES} (${PDAL_VERSION})")
   ENDIF (NOT PDAL_FIND_QUIETLY)

   IF ((PDAL_VERSION_MAJOR EQUAL 1) AND (PDAL_VERSION_MINOR LESS 7))
      MESSAGE (FATAL_ERROR "PDAL version is too old (${PDAL_VERSION}). Use 1.7 or higher.")
   ENDIF()

ELSE (PDAL_FOUND)
   IF (PDAL_FIND_REQUIRED)
     MESSAGE(FATAL_ERROR "Could not find PDAL")
   ELSE (PDAL_FIND_REQUIRED)
     IF (NOT PDAL_FIND_QUIETLY)
        MESSAGE(STATUS "Could not find PDAL")
     ENDIF (NOT PDAL_FIND_QUIETLY)
   ENDIF (PDAL_FIND_REQUIRED)

ENDIF (PDAL_FOUND)
