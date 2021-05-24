# Find O2
# ~~~~~~~~~
# Copyright (c) 2016, Monsanto Company, USA
# Author: Larry Shaffer, <lshaffer (at) boundlessgeo (dot) com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#
# CMake module to search for O2 OAuth 1/2 library from:
#    https://github.com/pipacs/o2
#
# If it's found it sets O2_FOUND to TRUE
# and following variables are set:
#    O2_INCLUDE_DIR
#    O2_LIBRARY
#    O2_LIBRARY_STATIC

IF (O2_INCLUDE_DIR AND (O2_LIBRARY OR O2_LIBRARY_STATIC))
   SET(O2_FOUND TRUE)

ELSE (O2_INCLUDE_DIR AND (O2_LIBRARY OR O2_LIBRARY_STATIC))

  FIND_PATH(O2_INCLUDE_DIR o2.h
    PATHS
    /usr/include
    /usr/local/include
    "$ENV{LIB_DIR}/include"
    $ENV{INCLUDE}
    PATH_SUFFIXES o2
    )
  FIND_LIBRARY(O2_LIBRARY NAMES o2
    PATHS
    /usr/local/lib
    /usr/lib
    "$ENV{LIB_DIR}/lib"
    "$ENV{LIB}"
    )
  FIND_LIBRARY(O2_LIBRARY_STATIC NAMES libo2.a libo2_static.a o2_static
    PATHS
    /usr/local/lib
    /usr/lib
    "$ENV{LIB_DIR}/lib"
    "$ENV{LIB}"
    )

  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(O2 DEFAULT_MSG O2_LIBRARY O2_INCLUDE_DIR)

ENDIF (O2_INCLUDE_DIR AND (O2_LIBRARY OR O2_LIBRARY_STATIC))

IF (O2_FOUND)
   IF (NOT O2_FIND_QUIETLY)
      MESSAGE(STATUS "Found O2: ${O2_LIBRARY} ${O2_LIBRARY_STATIC}")
   ENDIF (NOT O2_FIND_QUIETLY)
ELSE (O2_FOUND)
   IF (O2_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "Could not find O2")
   ENDIF (O2_FIND_REQUIRED)
ENDIF (O2_FOUND)
