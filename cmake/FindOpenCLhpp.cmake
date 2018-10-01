# Find OpenCLhpp
# ~~~~~~~~~~~~~~~
# CMake module to search for OpenCL headers for C++ bindings from:
#    https://github.com/KhronosGroup/OpenCL-CLHPP
#
# Specifically, it finds the cpl2 header.
#
# If it's found it sets OPENCL_HPP_FOUND to TRUE
# and following variables are set:
#    OPENCL_HPP_INCLUDE_DIR
#
# Copyright (c) 2018, Boundless Spatial
# Author: Larry Shaffer <lshaffer (at) boundlessgeo (dot) com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if (OPENCL_HPP_INCLUDE_DIR)
    SET(OPENCL_HPP_FOUND TRUE)
else ()
    find_path(OPENCL_HPP_INCLUDE_DIR
        NAMES CL/cl2.hpp
        PATHS
        ${LIB_DIR}/include
        "$ENV{LIB_DIR}/include"
        $ENV{INCLUDE}
        /usr/local/include
        /usr/include
    )
    if (OPENCL_HPP_INCLUDE_DIR)
        SET(OPENCL_HPP_FOUND TRUE)
    else ()
        SET(OPENCL_HPP_FOUND FALSE)
    endif ()
endif ()


if (OPENCL_HPP_FOUND)
    if (NOT OPENCLHPP_FIND_QUIETLY)
        message(STATUS "Found OpenCL C++ headers: ${OPENCL_HPP_INCLUDE_DIR}")
    endif ()
else ()
    if (OPENCLHPP_FIND_REQUIRED)
      message(FATAL_ERROR "Could not find OpenCL C++ headers")
    endif ()
endif ()

mark_as_advanced(OPENCL_HPP_INCLUDE_DIR)
