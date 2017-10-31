# Find QJSON - JSON handling library for Qt
#
# Copyright (c) 2016, Monsanto Company, USA
# Author: Larry Shaffer, <lshaffer (at) boundlessgeo (dot) com>
#
# Culled from QJson 0.7.1 release
#
# This module defines
#  QJSON_FOUND - whether the qsjon library was found
#  QJSON_LIBRARIES - the qjson library
#  QJSON_INCLUDE_DIR - the include path of the qjson library
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if (QJSON_INCLUDE_DIR AND QJSON_LIBRARIES)

  # Already in cache
  set (QJSON_FOUND TRUE)

else (QJSON_INCLUDE_DIR AND QJSON_LIBRARIES)

#  if (NOT WIN32)
#    # use pkg-config to get the values of QJSON_INCLUDE_DIRS
#    # and QJSON_LIBRARY_DIRS to add as hints to the find commands.
#    include (FindPkgConfig)
#    pkg_check_modules (QJSON REQUIRED QJson>=0.8)
#  endif (NOT WIN32)

  find_library (QJSON_LIBRARIES
    NAMES qjson qjson-qt5
    PATHS
    ${QJSON_LIBRARY_DIRS}
    "$ENV{OSGEO4W_ROOT}/lib"
    /usr/local/lib
    /usr/lib
    "$ENV{LIB_DIR}/lib"
    "$ENV{LIB}"
    ${LIB_INSTALL_DIR}
    ${KDE4_LIB_DIR}
  )

  find_path (QJSON_INCLUDE_DIR
    NAMES qjson/parser.h
    PATHS
    ${QJSON_INCLUDE_DIRS}
    "$ENV{OSGEO4W_ROOT}/include"
    /usr/include
    /usr/local/include
    "$ENV{LIB_DIR}/include"
    $ENV{INCLUDE}
    ${INCLUDE_INSTALL_DIR}
    ${KDE4_INCLUDE_DIR}
  )

  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(QJSON DEFAULT_MSG QJSON_LIBRARIES QJSON_INCLUDE_DIR)

endif (QJSON_INCLUDE_DIR AND QJSON_LIBRARIES)

if (QJSON_FOUND)
   if (NOT QJSON_FIND_QUIETLY)
      message(STATUS "Found QJson: ${QJSON_LIBRARIES}")
   endif (NOT QJSON_FIND_QUIETLY)
else (QJSON_FOUND)
  if (NOT QJSON_FIND_QUIETLY)
     message(FATAL_ERROR "Could not find QJson")
  endif (NOT QJSON_FIND_QUIETLY)
endif (QJSON_FOUND)
