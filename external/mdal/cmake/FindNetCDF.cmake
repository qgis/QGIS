# - Find NetCDF
# Find the native NetCDF includes and library
# Copyright (c) 2018, Peter Petrik <zilolv at gmail dot com>
#
# This module returns these variables for the rest of the project to use.
#
#  NETCDF_FOUND          - True if NetCDF found including required interfaces (see below)
#  NETCDF_LIBRARY        - All netcdf related libraries
#  NETCDF_INCLUDE_DIR    - All directories to include

IF (NETCDF_INCLUDE_DIR AND NETCDF_LIBRARY)
  # Already in cache, be silent
  SET (NETCDF_FIND_QUIETLY TRUE)
ENDIF ()

FIND_PACKAGE(PkgConfig)
PKG_CHECK_MODULES(PC_NETCDF QUIET netcdf)
SET(NETCDF_DEFINITIONS ${PC_NETCDF_CFLAGS_OTHER})

FIND_PATH (NETCDF_INCLUDE_DIR netcdf.h 
           HINTS ${PC_NETCDF_INCLUDEDIR} ${PC_NETCDF_INCLUDE_DIRS} ${NETCDF_PREFIX}/include
           PATH_SUFFIXES libnetcdf )
           
FIND_LIBRARY (NETCDF_LIBRARY 
              NAMES netcdf libnetcdf 
              HINTS HINTS ${PC_NETCDF_LIBDIR} ${PC_NETCDF_LIBRARY_DIRS} ${NETCDF_PREFIX}/lib)

INCLUDE (FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS (NetCDF
  DEFAULT_MSG NETCDF_LIBRARY NETCDF_INCLUDE_DIR)
