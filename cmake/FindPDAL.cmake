# Find PDAL
# ~~~~~~~~~~
# Copyright (c) 2020, Peter Petrik <zilolv at gmail.com>
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#
# CMake module to search for PDAL library
#
# If it's found it sets PDAL_FOUND to TRUE
# and adds the following variable containing library target(s):
#    PDAL_LIBRARIES

# FIND_PATH and FIND_LIBRARY normally search standard locations
# before the specified paths. To search non-standard paths first,
# FIND_* is invoked first with specified paths and NO_DEFAULT_PATH
# and then again with no specified paths to search the default
# locations. When an earlier FIND_* succeeds, subsequent FIND_*s
# searching for the same item do nothing.

find_package(PDAL CONFIG)

if(PDAL_FOUND)
  if(NOT PDAL_FIND_QUIETLY)
    message(STATUS "Found PDAL: ${PDAL_LIBRARIES} (${PDAL_VERSION})")
  endif()

  if(PDAL_VERSION VERSION_LESS "1.7.0")
    message(FATAL_ERROR "PDAL version is too old (${PDAL_VERSION}). Use 1.7 or higher.")
  endif()

  if(MSVC)
    foreach(PDAL_TARG ${PDAL_LIBRARIES})
      target_compile_definitions(${PDAL_TARG} INTERFACE WIN32_LEAN_AND_MEAN)
    endforeach()
  endif()

  return()
endif()

# Fallback for systems where PDAL's config-file package is not present.
# It is not adapted for PDAL 2.6+ (where pdal_util library is removed).
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
   SET(PDAL_LIBRARIES pdalcpp pdal_util)
ENDIF (PDAL_INCLUDE_DIR AND PDAL_CPP_LIBRARY AND PDAL_UTIL_LIBRARY AND PDAL_BIN)

IF (PDAL_FOUND)
    # extract PDAL version
    execute_process(COMMAND ${PDAL_BIN} --version
        OUTPUT_STRIP_TRAILING_WHITESPACE OUTPUT_VARIABLE PDAL_VERSION_OUT )
    STRING(REGEX REPLACE "^.*([0-9]+)\\.([0-9]+)\\.([0-9]+).*$" "\\1" PDAL_VERSION_MAJOR "${PDAL_VERSION_OUT}")
    STRING(REGEX REPLACE "^.*([0-9]+)\\.([0-9]+)\\.([0-9]+).*$" "\\2" PDAL_VERSION_MINOR "${PDAL_VERSION_OUT}")
    STRING(REGEX REPLACE "^.*([0-9]+)\\.([0-9]+)\\.([0-9]+).*$" "\\3" PDAL_VERSION_PATCH "${PDAL_VERSION_OUT}")
    STRING(CONCAT PDAL_VERSION ${PDAL_VERSION_MAJOR} "." ${PDAL_VERSION_MINOR} "." ${PDAL_VERSION_PATCH})

   IF (NOT PDAL_FIND_QUIETLY)
      MESSAGE(STATUS "Found PDAL: ${PDAL_LIBRARIES} (${PDAL_VERSION})")
   ENDIF (NOT PDAL_FIND_QUIETLY)

   IF ((PDAL_VERSION_MAJOR EQUAL 1) AND (PDAL_VERSION_MINOR LESS 7))
      MESSAGE (FATAL_ERROR "PDAL version is too old (${PDAL_VERSION}). Use 1.7 or higher.")
   ENDIF()

   add_library(pdalcpp UNKNOWN IMPORTED)
   target_link_libraries(pdalcpp INTERFACE ${PDAL_CPP_LIBRARY})
   target_include_directories(pdalcpp INTERFACE ${PDAL_INCLUDE_DIR})
   set_target_properties(pdalcpp PROPERTIES IMPORTED_LOCATION ${PDAL_CPP_LIBRARY})

   add_library(pdal_util UNKNOWN IMPORTED)
   target_link_libraries(pdal_util INTERFACE ${PDAL_UTIL_LIBRARY})
   target_include_directories(pdal_util INTERFACE ${PDAL_INCLUDE_DIR})
   set_target_properties(pdal_util PROPERTIES IMPORTED_LOCATION ${PDAL_UTIL_LIBRARY})

   if(MSVC)
     target_compile_definitions(pdalcpp INTERFACE WIN32_LEAN_AND_MEAN)
     target_compile_definitions(pdal_util INTERFACE WIN32_LEAN_AND_MEAN)
   endif()

ELSE (PDAL_FOUND)
   IF (PDAL_FIND_REQUIRED)
     MESSAGE(FATAL_ERROR "Could not find PDAL")
   ELSE (PDAL_FIND_REQUIRED)
     IF (NOT PDAL_FIND_QUIETLY)
        MESSAGE(STATUS "Could not find PDAL")
     ENDIF (NOT PDAL_FIND_QUIETLY)
   ENDIF (PDAL_FIND_REQUIRED)

ENDIF (PDAL_FOUND)
