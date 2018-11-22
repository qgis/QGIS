# Find MDAL
# ~~~~~~~~~
# Copyright (c) 2018, Peter Petrik <zilolv at gmail dot com>
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#
#
# Once run this will define: 
#  MDAL_FOUND - System has MDAL
#  MDAL_INCLUDE_DIRS - The MDAL include directories
#  MDAL_LIBRARIES - The libraries needed to use MDAL
#  MDAL_DEFINITIONS - Compiler switches required for using MDAL

FIND_PACKAGE(PkgConfig)
PKG_CHECK_MODULES(PC_MDAL QUIET libmdal)
SET(MDAL_DEFINITIONS ${PC_MDAL_CFLAGS_OTHER})

FIND_PATH(MDAL_INCLUDE_DIR mdal.h
          HINTS ${PC_MDAL_INCLUDEDIR} ${PC_MDAL_INCLUDE_DIRS} ${MDAL_PREFIX}/include
          PATH_SUFFIXES libmdal )

FIND_LIBRARY(MDAL_LIBRARY NAMES mdal libmdal
             HINTS ${PC_MDAL_LIBDIR} ${PC_MDAL_LIBRARY_DIRS} ${MDAL_PREFIX}/lib)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(MDAL DEFAULT_MSG
                                  MDAL_LIBRARY MDAL_INCLUDE_DIR)

MARK_AS_ADVANCED(MDAL_INCLUDE_DIR MDAL_LIBRARY )

SET(MDAL_LIBRARIES ${MDAL_LIBRARY} )
SET(MDAL_INCLUDE_DIRS ${MDAL_INCLUDE_DIR} )
