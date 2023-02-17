# CMake module to search for laz-perf
#
# Once done this will define
#
#  LazPerf_FOUND - system has the zip library
#  LazPerf_INCLUDE_DIRS - the zip include directories
#
# Copyright (c) 2020, Peter Petrik, <zilolv@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

FIND_PATH(LazPerf_INCLUDE_DIR
  lazperf/lazperf.hpp
  "$ENV{LIB_DIR}/include"
  "$ENV{INCLUDE}"
  /usr/local/include
  /usr/include
  NO_DEFAULT_PATH
)

FIND_LIBRARY(LazPerf_LIBRARY
  NAMES lazperf PATHS
  "$ENV{LIB_DIR}/lib"
  /usr/local/lib
  /usr/lib
)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(LazPerf DEFAULT_MSG LazPerf_LIBRARY LazPerf_INCLUDE_DIR)

MARK_AS_ADVANCED(LazPerf_INCLUDE_DIR)
MARK_AS_ADVANCED(LazPerf_LIBRARY)

IF (LazPerf_FOUND)
  MESSAGE(STATUS "Found LAZperf: ${LazPerf_LIBRARY}")
ENDIF (LazPerf_FOUND)
