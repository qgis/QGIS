# Find Expat
# ~~~~~~~~~~
# Copyright (c) 2007, Martin Dobias <wonder.sk at gmail.com>
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#
# CMake module to search for Expat library
# (library for parsing XML files)
#
# If it's found it sets EXPAT_FOUND to TRUE
# and following variables are set:
#    EXPAT_INCLUDE_DIR
#    EXPAT_LIBRARY

# FIND_PATH and FIND_LIBRARY normally search standard locations
# before the specified paths. To search non-standard paths first,
# FIND_* is invoked first with specified paths and NO_DEFAULT_PATH
# and then again with no specified paths to search the default
# locations. When an earlier FIND_* succeeds, subsequent FIND_*s
# searching for the same item do nothing. 
FIND_PATH(EXPAT_INCLUDE_DIR expat.h
  "$ENV{LIB_DIR}/include/"
  "$ENV{LIB_DIR}/include/expat"
  c:/msys/local/include
  NO_DEFAULT_PATH
  )
FIND_PATH(EXPAT_INCLUDE_DIR expat.h)
#libexpat needed for msvc version
FIND_LIBRARY(EXPAT_LIBRARY NAMES expat libexpat PATHS 
  "$ENV{LIB_DIR}/lib"
  c:/msys/local/lib
  NO_DEFAULT_PATH
  )
FIND_LIBRARY(EXPAT_LIBRARY NAMES expat libexpat)

IF (EXPAT_INCLUDE_DIR AND EXPAT_LIBRARY)
   SET(EXPAT_FOUND TRUE)
ENDIF (EXPAT_INCLUDE_DIR AND EXPAT_LIBRARY)


IF (EXPAT_FOUND)

   IF (NOT EXPAT_FIND_QUIETLY)
      MESSAGE(STATUS "Found Expat: ${EXPAT_LIBRARY}")
   ENDIF (NOT EXPAT_FIND_QUIETLY)

ELSE (EXPAT_FOUND)

   IF (EXPAT_FIND_REQUIRED)
     MESSAGE(FATAL_ERROR "Could not find Expat")
   ELSE (EXPAT_FIND_REQUIRED)
     IF (NOT EXPAT_FIND_QUIETLY)
        MESSAGE(STATUS "Could not find Expat")
     ENDIF (NOT EXPAT_FIND_QUIETLY)
   ENDIF (EXPAT_FIND_REQUIRED)

ENDIF (EXPAT_FOUND)
