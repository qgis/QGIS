# Find Proj
# ~~~~~~~~~
# Copyright (c) 2007, Martin Dobias <wonder.sk at gmail.com>
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#
# CMake module to search for Proj library
#
# If it's found it sets PROJ_FOUND to TRUE
# and adds the following target
#
#   PROJ::proj

# FIND_PATH and FIND_LIBRARY normally search standard locations
# before the specified paths. To search non-standard paths first,
# FIND_* is invoked first with specified paths and NO_DEFAULT_PATH
# and then again with no specified paths to search the default
# locations. When an earlier FIND_* succeeds, subsequent FIND_*s
# searching for the same item do nothing.

find_package(PROJ CONFIG)

# Fallback for systems where proj-targets.cmake is not present.
if(NOT PROJ_FOUND)
  # try to use framework on mac
  # want clean framework path, not unix compatibility path
  IF (APPLE)
    IF (CMAKE_FIND_FRAMEWORK MATCHES "FIRST"
        OR CMAKE_FRAMEWORK_PATH MATCHES "ONLY"
        OR NOT CMAKE_FIND_FRAMEWORK)
      SET (CMAKE_FIND_FRAMEWORK_save ${CMAKE_FIND_FRAMEWORK} CACHE STRING "" FORCE)
      SET (CMAKE_FIND_FRAMEWORK "ONLY" CACHE STRING "" FORCE)
      FIND_LIBRARY(PROJ_LIBRARY PROJ)
      IF (PROJ_LIBRARY)
        # FIND_PATH doesn't add "Headers" for a framework
        SET (PROJ_INCLUDE_DIR ${PROJ_LIBRARY}/Headers CACHE PATH "Path to a file.")
      ENDIF (PROJ_LIBRARY)
      SET (CMAKE_FIND_FRAMEWORK ${CMAKE_FIND_FRAMEWORK_save} CACHE STRING "" FORCE)
    ENDIF ()
  ENDIF (APPLE)

  FIND_PATH(PROJ_INCLUDE_DIR proj.h
    "$ENV{INCLUDE}"
    "$ENV{LIB_DIR}/include"
  )
  
  FIND_LIBRARY(PROJ_LIBRARY NAMES proj_i proj PATHS
    "$ENV{LIB}"
    "$ENV{LIB_DIR}/lib"
    )
  
  IF (PROJ_INCLUDE_DIR AND PROJ_LIBRARY)
     SET(PROJ_FOUND TRUE)
  ENDIF (PROJ_INCLUDE_DIR AND PROJ_LIBRARY)
  
  IF (PROJ_FOUND)
     IF (EXISTS ${PROJ_INCLUDE_DIR}/proj.h AND EXISTS ${PROJ_INCLUDE_DIR}/proj_experimental.h)
       FILE(READ ${PROJ_INCLUDE_DIR}/proj.h proj_version)
       STRING(REGEX REPLACE "^.*PROJ_VERSION_MAJOR +([0-9]+).*$" "\\1" PROJ_VERSION_MAJOR "${proj_version}")
       STRING(REGEX REPLACE "^.*PROJ_VERSION_MINOR +([0-9]+).*$" "\\1" PROJ_VERSION_MINOR "${proj_version}")
       STRING(REGEX REPLACE "^.*PROJ_VERSION_PATCH +([0-9]+).*$" "\\1" PROJ_VERSION_PATCH "${proj_version}")
       STRING(CONCAT PROJ_VERSION_STR "(" ${PROJ_VERSION_MAJOR} "." ${PROJ_VERSION_MINOR} "." ${PROJ_VERSION_PATCH} ")")
     ENDIF(EXISTS ${PROJ_INCLUDE_DIR}/proj.h AND EXISTS ${PROJ_INCLUDE_DIR}/proj_experimental.h)
     IF (NOT PROJ_FIND_QUIETLY)
       MESSAGE(STATUS "Found Proj: ${PROJ_LIBRARY} version ${PROJ_VERSION_MAJOR} ${PROJ_VERSION_STR}")
     ENDIF (NOT PROJ_FIND_QUIETLY)

     SET(PROJ_VERSION ${PROJ_VERSION_MAJOR}.${PROJ_VERSION_MINOR}.${PROJ_VERSION_PATCH})
  
     INCLUDE_DIRECTORIES(BEFORE SYSTEM ${PROJ_INCLUDE_DIR})

     add_library(PROJ::proj UNKNOWN IMPORTED)
     target_link_libraries(PROJ::proj INTERFACE ${PROJ_LIBRARY})
     target_include_directories(PROJ::proj INTERFACE ${PROJ_INCLUDE_DIR})
     set_target_properties(PROJ::proj PROPERTIES IMPORTED_LOCATION ${PROJ_LIBRARY})
  
  ELSE (PROJ_FOUND)
  
     IF (PROJ_FIND_REQUIRED)
        MESSAGE(FATAL_ERROR "Could not find Proj")
     ENDIF (PROJ_FIND_REQUIRED)
  
  ENDIF (PROJ_FOUND)
endif()
