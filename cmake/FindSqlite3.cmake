# Find Sqlite3
# ~~~~~~~~~~~~
# Copyright (c) 2007, Martin Dobias <wonder.sk at gmail.com>
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#
# CMake module to search for Sqlite3 library
#
# If it's found it sets SQLITE3_FOUND to TRUE
# and following variables are set:
#    SQLITE3_INCLUDE_DIR
#    SQLITE3_LIBRARY


# FIND_PATH and FIND_LIBRARY normally search standard locations
# before the specified paths. To search non-standard paths first,
# FIND_* is invoked first with specified paths and NO_DEFAULT_PATH
# and then again with no specified paths to search the default
# locations. When an earlier FIND_* succeeds, subsequent FIND_*s
# searching for the same item do nothing. 

# try to use framework on mac
IF (APPLE)
  SET (SQLITE3_MAC_INC_PATH /Library/Frameworks/SQLite3.framework/Headers)
ENDIF (APPLE)

FIND_PATH(SQLITE3_INCLUDE_DIR sqlite3.h
  "$ENV{LIB_DIR}/include"
  "$ENV{LIB_DIR}/include/sqlite"
  ${SQLITE3_MAC_INC_PATH}
  #mingw
  c:/msys/local/include
  NO_DEFAULT_PATH
  )
FIND_PATH(SQLITE3_INCLUDE_DIR sqlite3.h)

FIND_LIBRARY(SQLITE3_LIBRARY NAMES sqlite3 sqlite3_i PATHS
  "$ENV{LIB_DIR}/lib"
  #mingw
  c:/msys/local/lib
  NO_DEFAULT_PATH
  )
FIND_LIBRARY(SQLITE3_LIBRARY NAMES sqlite3)

IF (SQLITE3_INCLUDE_DIR AND SQLITE3_LIBRARY)
   SET(SQLITE3_FOUND TRUE)
ENDIF (SQLITE3_INCLUDE_DIR AND SQLITE3_LIBRARY)


IF (SQLITE3_FOUND)

   IF (NOT SQLITE3_FIND_QUIETLY)
      MESSAGE(STATUS "Found Sqlite3: ${SQLITE3_LIBRARY}")
   ENDIF (NOT SQLITE3_FIND_QUIETLY)

ELSE (SQLITE3_FOUND)

   IF (SQLITE3_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "Could not find Sqlite3")
   ENDIF (SQLITE3_FIND_REQUIRED)

ENDIF (SQLITE3_FOUND)
