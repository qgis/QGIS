# Macros/functions for debugging CMake
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#
# Copyright (c) 2016, Larry Shaffer, <lshaffer (at) boundlessgeo (dot) com>>
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.


# Dump current CMake variables to file
#
# Usage:
# INCLUDE(CMakeDebugMacros)
# DUMP_CMAKE_VARS()  or  DUMP_CMAKE_VARS("regex")
#
# regex: optional ARGV0 regular expression for filtering output variable names
#
# Outputs the result relative to the current CMake file being processed and
# writes to a file with name "<file-basename>_cmake-vars.txt" to the current
# build (binary) directory
#
function(DUMP_CMAKE_VARS)

  get_filename_component(_basename ${CMAKE_CURRENT_LIST_FILE} NAME_WE)
  set(_out "${CMAKE_CURRENT_BINARY_DIR}/${_basename}_cmake-vars.txt")

  set(_cmake_vars "")
  get_cmake_property(_varNames VARIABLES)
  foreach(_varName ${_varNames})
    if(ARGV0)
      string(REGEX MATCH "${ARGV0}" _match "${_varName}")
      if(_match)
        set(_cmake_vars "${_cmake_vars}\n\n${_varName}=${${_varName}}")
      endif()
    else()
      set(_cmake_vars "${_cmake_vars}\n\n${_varName}=${${_varName}}")
    endif()
  endforeach()

  message(STATUS "Dumping current CMake variables to ...\n  ${_out}")
  file(WRITE "${_out}" "${_cmake_vars}")

endfunction(DUMP_CMAKE_VARS)
