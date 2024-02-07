# Find poly2tri
# ~~~~~~~~~~~~~
# Copyright (c) 2020, Peter Petrik <zilolv at gmail dot com>
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#
#
# Once run this will define: 
#  poly2tri_FOUND - System has poly2tri
#  poly2tri::poly2tri - Target

find_package(poly2tri CONFIG)
if(NOT poly2tri_FOUND)
  find_path(poly2tri_INCLUDE_DIR poly2tri.h
            HINTS $ENV{LIB_DIR}/include)
  
  find_library(poly2tri_LIBRARY NAMES poly2tri libpoly2tri
               HINTS $ENV{LIB_DIR}/lib)
  
  include(FindPackageHandleStandardArgs)
  
  find_package_handle_standard_args(poly2tri DEFAULT_MSG
                                    poly2tri_LIBRARY poly2tri_INCLUDE_DIR)

  
  add_library(poly2tri::poly2tri UNKNOWN IMPORTED)
  target_link_libraries(poly2tri::poly2tri INTERFACE ${poly2tri_LIBRARY})
  target_include_directories(poly2tri::poly2tri INTERFACE ${poly2tri_INCLUDE_DIR})
  set_target_properties(poly2tri::poly2tri PROPERTIES IMPORTED_LOCATION ${poly2tri_LIBRARY})
  mark_as_advanced(poly2tri_INCLUDE_DIR poly2tri_LIBRARY)
endif()
