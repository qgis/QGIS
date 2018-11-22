# Find Libtasn1
# ~~~~~~~~~~~~~~~
# CMake module to search for Libtasn1 ASN.1 library and header(s) from:
#    https://www.gnu.org/software/libtasn1/
#
# If it's found it sets LIBTASN1_FOUND to TRUE
# and following variables are set:
#    LIBTASN1_INCLUDE_DIR
#    LIBTASN1_LIBRARY
#
# Copyright (c) 2017, Boundless Spatial
# Author: Larry Shaffer <lshaffer (at) boundlessgeo (dot) com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.


find_path(LIBTASN1_INCLUDE_DIR
    NAMES libtasn1.h
    PATHS
    ${LIB_DIR}/include
    "$ENV{LIB_DIR}/include"
    $ENV{INCLUDE}
    /usr/local/include
    /usr/include
)

find_library(LIBTASN1_LIBRARY
    NAMES tasn1
    PATHS
    ${LIB_DIR}
    "$ENV{LIB_DIR}"
    $ENV{LIB}
    /usr/local/lib
    /usr/lib
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  Libtasn1
  REQUIRED_VARS LIBTASN1_INCLUDE_DIR LIBTASN1_LIBRARY
  FOUND_VAR LIBTASN1_FOUND
)

mark_as_advanced(LIBTASN1_INCLUDE_DIR LIBTASN1_LIBRARY)
