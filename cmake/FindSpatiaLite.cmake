# Find SpatiaLite
# ~~~~~~~~~~~~~~~
# Copyright (c) 2009, Sandro Furieri <a.furieri at lqt.it>
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#
# CMake module to search for SpatiaLite library
#
# If it's found it sets SPATIALITE_FOUND to TRUE
# and adds the following target
#
#   spatialite::spatialite

find_package(PkgConfig)
if(PkgConfig_FOUND)
  pkg_search_module(PC_SPATIALITE IMPORTED_TARGET spatialite)
endif()

if(PC_SPATIALITE_FOUND)
  if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.18)
    add_library(spatialite::spatialite ALIAS PkgConfig::PC_SPATIALITE)
  else()
    set(SPATIALITE_INCLUDE_DIR "${PC_SPATIALITE_INCLUDE_DIRS}" CACHE STRING "")
    set(SPATIALITE_LIBRARY "${PC_SPATIALITE_LIBRARIES}" CACHE STRING "")
    if(NOT TARGET spatialite::spatialite)
      add_library(spatialite::spatialite INTERFACE IMPORTED)
      set_target_properties(spatialite::spatialite PROPERTIES INTERFACE_LINK_LIBRARIES PkgConfig::PC_SPATIALITE)
    endif()
  endif()
  set(SPATIALITE_FOUND TRUE)
else()
  # Fallback for systems without PkgConfig, e.g. OSGeo4W

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
  IF (APPLE AND NOT IOS)
    IF (CMAKE_FIND_FRAMEWORK MATCHES "FIRST"
        OR CMAKE_FRAMEWORK_PATH MATCHES "ONLY"
        OR NOT CMAKE_FIND_FRAMEWORK)
      SET (CMAKE_FIND_FRAMEWORK_save ${CMAKE_FIND_FRAMEWORK} CACHE STRING "" FORCE)
      SET (CMAKE_FIND_FRAMEWORK "ONLY" CACHE STRING "" FORCE)
      FIND_PATH(SPATIALITE_INCLUDE_DIR SQLite3/spatialite.h)
      # if no SpatiaLite header, we don't want SQLite find below to succeed
      IF (SPATIALITE_INCLUDE_DIR)
        FIND_LIBRARY(SPATIALITE_LIBRARY SQLite3)
        # FIND_PATH doesn't add "Headers" for a framework
        SET (SPATIALITE_INCLUDE_DIR ${SPATIALITE_LIBRARY}/Headers CACHE PATH "Path to a file." FORCE)
      ENDIF (SPATIALITE_INCLUDE_DIR)
      SET (CMAKE_FIND_FRAMEWORK ${CMAKE_FIND_FRAMEWORK_save} CACHE STRING "" FORCE)
    ENDIF ()
  ENDIF (APPLE AND NOT IOS)
  
  FIND_PATH(SPATIALITE_INCLUDE_DIR spatialite.h
    /usr/include
    "$ENV{INCLUDE}"
    "$ENV{LIB_DIR}/include"
    "$ENV{LIB_DIR}/include/spatialite"
    )
  
  FIND_LIBRARY(SPATIALITE_LIBRARY NAMES spatialite_i spatialite PATHS
    /usr/lib
    $ENV{LIB}
    $ENV{LIB_DIR}/lib
    )
  
  IF (SPATIALITE_INCLUDE_DIR AND SPATIALITE_LIBRARY)
     SET(SPATIALITE_FOUND TRUE)
  ENDIF (SPATIALITE_INCLUDE_DIR AND SPATIALITE_LIBRARY)
  
  
  IF (SPATIALITE_FOUND)
     add_library(spatialite::spatialite UNKNOWN IMPORTED)
     target_link_libraries(spatialite::spatialite INTERFACE ${SPATIALITE_LIBRARY})
     target_include_directories(spatialite::spatialite INTERFACE ${SPATIALITE_INCLUDE_DIR})
     set_target_properties(spatialite::spatialite PROPERTIES IMPORTED_LOCATION ${SPATIALITE_LIBRARY})
  
     IF (NOT SPATIALITE_FIND_QUIETLY)
        MESSAGE(STATUS "Found SpatiaLite: ${SPATIALITE_LIBRARY}")
     ENDIF (NOT SPATIALITE_FIND_QUIETLY)
  
     IF(APPLE)
       # no extra LDFLAGS used in link test, may fail in OS X SDK
       SET(CMAKE_REQUIRED_LIBRARIES "-F/Library/Frameworks" ${CMAKE_REQUIRED_LIBRARIES})
     ENDIF(APPLE)
  
  ELSE (SPATIALITE_FOUND)
  
     IF (SPATIALITE_FIND_REQUIRED)
       MESSAGE(FATAL_ERROR "Could not find SpatiaLite. Include: ${SPATIALITE_INCLUDE_DIR} Library: ${SPATIALITE_LIBRARY}")
     ENDIF (SPATIALITE_FIND_REQUIRED)
  
  ENDIF (SPATIALITE_FOUND)
endif()
