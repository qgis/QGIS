# Find earcut
# ~~~~~~~~~~~~~
# Copyright (c) 2025, Dominik Cindrić <viper dot miniq at gmail dot com>
# Redistribution and use is allowed according to the terms of the ISC license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#
#
# Once run this will define: 
#  earcut_FOUND - System has earcut
#  earcut::earcut - Target

find_package(earcut CONFIG)
if(NOT earcut_FOUND)
  find_path(earcut_INCLUDE_DIR mapbox/earcut.hpp
            HINTS $ENV{LIB_DIR}/include)

  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(earcut DEFAULT_MSG earcut_INCLUDE_DIR)

  add_library(earcut::earcut INTERFACE IMPORTED)
  target_include_directories(earcut::earcut INTERFACE ${earcut_INCLUDE_DIR})
  mark_as_advanced(earcut_INCLUDE_DIR)
endif()
