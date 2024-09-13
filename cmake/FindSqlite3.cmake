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

# We ensure consistency between the target defined by this file
# and the official CMake's FindSQLite3.cmake
# https://cmake.org/cmake/help/latest/module/FindSQLite3.html
if(SQLITE3_FOUND)
  if(NOT SQLite3_FOUND OR NOT TARGET SQLite::SQLite3)
      message(FATAL_ERROR "Unconsistency between SQLite3 dependencies")
  endif()
  return()
endif()

# try to use framework on mac
# want clean framework path, not unix compatibility path
IF (APPLE AND NOT QGIS_MAC_DEPS_DIR)
  IF (CMAKE_FIND_FRAMEWORK MATCHES "FIRST"
      OR CMAKE_FRAMEWORK_PATH MATCHES "ONLY"
      OR NOT CMAKE_FIND_FRAMEWORK)
    SET (CMAKE_FIND_FRAMEWORK_save ${CMAKE_FIND_FRAMEWORK} CACHE STRING "" FORCE)
    SET (CMAKE_FIND_FRAMEWORK "ONLY" CACHE STRING "" FORCE)
    #FIND_PATH(SQLITE3_INCLUDE_DIR SQLite3/sqlite3.h)
    FIND_LIBRARY(SQLITE3_LIBRARY SQLite3)
    IF (SQLITE3_LIBRARY)
      # FIND_PATH doesn't add "Headers" for a framework
      SET (SQLITE3_INCLUDE_DIR ${SQLITE3_LIBRARY}/Headers CACHE PATH "Path to a file.")
    ENDIF (SQLITE3_LIBRARY)
    SET (CMAKE_FIND_FRAMEWORK ${CMAKE_FIND_FRAMEWORK_save} CACHE STRING "" FORCE)
  ENDIF ()
ENDIF (APPLE AND NOT QGIS_MAC_DEPS_DIR)

FIND_PATH(SQLITE3_INCLUDE_DIR sqlite3.h
  "$ENV{LIB_DIR}/include"
  "$ENV{LIB_DIR}/include/sqlite"
  "$ENV{INCLUDE}"
  NO_DEFAULT_PATH
)
FIND_PATH(SQLITE3_INCLUDE_DIR sqlite3.h)

FIND_LIBRARY(SQLITE3_LIBRARY NAMES sqlite3_i sqlite3 PATHS
  "$ENV{LIB_DIR}/lib"
  "$ENV{LIB}/lib"
  NO_DEFAULT_PATH
)
FIND_LIBRARY(SQLITE3_LIBRARY NAMES sqlite3_i sqlite3)

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

# Create the imported target following the official CMake's FindSQLite3.cmake
if(SQLITE3_FOUND)
    set(SQLite3_FOUND TRUE)
    set(SQLite3_INCLUDE_DIRS ${SQLITE3_INCLUDE_DIR})
    set(SQLite3_LIBRARIES ${SQLITE3_LIBRARY})
    if(NOT TARGET SQLite::SQLite3)
        add_library(SQLite::SQLite3 UNKNOWN IMPORTED)
        set_target_properties(SQLite::SQLite3 PROPERTIES
            IMPORTED_LOCATION             "${SQLITE3_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${SQLITE3_INCLUDE_DIR}")
    else()
        message(FATAL_ERROR "SQLite::SQLite3 target should not have been defined at this point.")
    endif()
endif()
