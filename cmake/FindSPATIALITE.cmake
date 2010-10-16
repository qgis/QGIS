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


# FIND_PATH and FIND_LIBRARY normally search standard locations
# before the specified paths. To search non-standard paths first,
# FIND_* is invoked first with specified paths and NO_DEFAULT_PATH
# and then again with no specified paths to search the default
# locations. When an earlier FIND_* succeeds, subsequent FIND_*s
# searching for the same item do nothing. 

FIND_PATH(SPATIALITE_INCLUDE_DIR spatialite.h
  "$ENV{LIB_DIR}/include"
  "$ENV{LIB_DIR}/include/spatialite"
  # try to use SQLite3 framework on mac, should be set from SQLite3 check
  ${SQLITE3_MAC_INC_PATH}
  #mingw
  c:/msys/local/include
  NO_DEFAULT_PATH
  )
FIND_PATH(SPATIALITE_INCLUDE_DIR spatialite.h)

FIND_LIBRARY(SPATIALITE_LIBRARY NAMES spatialite PATHS
  "$ENV{LIB_DIR}/lib"
  #mingw
  c:/msys/local/lib
  NO_DEFAULT_PATH
  )
FIND_LIBRARY(SPATIALITE_LIBRARY NAMES spatialite)

IF (SPATIALITE_INCLUDE_DIR AND SPATIALITE_LIBRARY)
   SET(SPATIALITE_FOUND TRUE)
ENDIF (SPATIALITE_INCLUDE_DIR AND SPATIALITE_LIBRARY)


IF (SPATIALITE_FOUND)

   IF (NOT SPATIALITE_FIND_QUIETLY)
      MESSAGE(STATUS "Found SpatiaLite: ${SPATIALITE_LIBRARY}")
   ENDIF (NOT SPATIALITE_FIND_QUIETLY)

ELSE (SPATIALITE_FOUND)

   IF (SPATIALITE_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "Could not find SpatiaLite")
   ENDIF (SPATIALITE_FIND_REQUIRED)

ENDIF (SPATIALITE_FOUND)
