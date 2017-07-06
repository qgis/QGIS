# CMake module to search for libzip
#
# Once done this will define
#
#  LIBZIP_FOUND - system has the zip library
#  LIBZIP_INCLUDE_DIR - the zip include directory
#  LIBZIP_LIBRARY - Link this to use the zip library
#
# Copyright (c) 2017, Paul Blottiere, <paul.blottiere@oslandia.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

FIND_PATH(LIBZIP_INCLUDE_DIR
  zip.h
  PATHS
  /usr/local/include
  /usr/include
  "$ENV{LIB_DIR}/include"
  "$ENV{INCLUDE}"
  )

FIND_LIBRARY(LIBZIP_LIBRARY
  NAMES zip
  PATHS
  /usr/local/lib
  /usr/lib
  "$ENV{LIB_DIR}/lib"
  "$ENV{LIB}"
  )

IF (LIBZIP_LIBRARY AND LIBZIP_INCLUDE_DIR)
  SET(LIBZIP_FOUND TRUE)
ENDIF (LIBZIP_LIBRARY AND LIBZIP_INCLUDE_DIR)

IF (LIBZIP_FOUND)
  MESSAGE(STATUS "Found libzip: ${LIBZIP_LIBRARY}")
ELSE (LIPZIP_FOUND)
  MESSAGE(FATAL_ERROR "Could not find libzip")
ENDIF (LIBZIP_FOUND)
