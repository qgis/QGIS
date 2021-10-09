# CMake module to search for libzstd
#
# Once done this will define
#
#  ZSTD_FOUND - system has the zip library
#  ZSTD_INCLUDE_DIRS - the zip include directories
#  ZSTD_LIBRARY - Link this to use the zip library
#
# Copyright (c) 2020, Peter Petrik, <zilolv@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

FIND_PATH(ZSTD_INCLUDE_DIR
  zstd.h
  PATHS
  "$ENV{LIB_DIR}/include"
  "$ENV{INCLUDE}"
  /usr/local/include
  /usr/include
  PATH_SUFFIXES
  zstd
)

FIND_LIBRARY(ZSTD_LIBRARY NAMES zstd PATHS "$ENV{LIB_DIR}/lib" "$ENV{LIB}" /usr/local/lib /usr/lib )

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(ZSTD DEFAULT_MSG
                                  ZSTD_LIBRARY ZSTD_INCLUDE_DIR)

MARK_AS_ADVANCED(ZSTD_LIBRARY ZSTD_INCLUDE_DIR)

IF (ZSTD_FOUND)
  MESSAGE(STATUS "Found ZSTD: ${ZSTD_LIBRARY}")
ENDIF (ZSTD_FOUND)
