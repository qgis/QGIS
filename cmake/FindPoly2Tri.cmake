# Find Poly2Tri
# ~~~~~~~~~
# Copyright (c) 2020, Peter Petrik <zilolv at gmail dot com>
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#
#
# Once run this will define: 
#  Poly2Tri_FOUND - System has Poly2Tri
#  Poly2Tri_INCLUDE_DIR - The Poly2Tri include directory
#  Poly2Tri_LIBRARY - The library needed to use Poly2Tri

find_path(Poly2Tri_INCLUDE_DIR poly2tri.h
          HINTS $ENV{LIB_DIR}/include)

find_library(Poly2Tri_LIBRARY NAMES poly2tri libpoly2tri
             HINTS $ENV{LIB_DIR}/lib)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(Poly2Tri DEFAULT_MSG
                                  Poly2Tri_LIBRARY Poly2Tri_INCLUDE_DIR)

mark_as_advanced( Poly2Tri_INCLUDE_DIR Poly2Tri_LIBRARY )
