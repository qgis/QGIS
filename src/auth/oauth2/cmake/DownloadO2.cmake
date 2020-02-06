# Download Module for o2 Library Source
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Copyright (c) March 2017, Boundless Spatial
# Author: Larry Shaffer <lshaffer (at) boundlessgeo (dot) com>
#
# Official o2 project source code repository: https://github.com/pipacs/o2
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

# NOTE: These is a temporary source code commit checkout, until a release with
#       recent commits is available via common platform packaging
set(_o2_commit 31ceafb3f0c3b605110ddd20aeebd3288504ee1f)
set(_o2_url "https://github.com/pipacs/o2/archive/${_o2_commit}.tar.gz")
set(_o2_dl_file "${CMAKE_BINARY_DIR}/o2-${_o2_commit}.tar.gz")
set(_o2_dl_sha1 94236af3c373927d778349cdbe89ff6112343cc9)
set(_o2_dl_timeout 45)
set(_o2_dl_inactive_timeout 30)

set(_o2_prefix "${CMAKE_BINARY_DIR}/o2-${_o2_commit}")

message(STATUS "Downloading or verifying o2 library source archive...")

file(DOWNLOAD ${_o2_url} ${_o2_dl_file}
   INACTIVITY_TIMEOUT ${_o2_dl_inactive_timeout}
   TIMEOUT ${_o2_dl_timeout}
   STATUS _o2_dl_status
   LOG _o2_dl_log
   #SHOW_PROGRESS
   EXPECTED_HASH SHA1=${_o2_dl_sha1}
   TLS_VERIFY on
   #TLS_CAINFO file
)

list(GET _o2_dl_status 0 _o2_dl_status_code)
list(GET _o2_dl_status 1 _o2_dl_status_string)

if(NOT "${_o2_dl_status_code}" STREQUAL "0")
  set(_o2_dl_log_file ${CMAKE_BINARY_DIR}/o2-download.log)
  set(_o2_dl_log_title "Error downloading or verifying o2 library source archive")
  set(_o2_dl_log_msg "${_o2_dl_log_title}
    from url=${_o2_url}
    to file =${_o2_dl_file}
    timeout =${_o2_dl_timeout} seconds
    status code: ${_o2_dl_status_code}
    status string: ${_o2_dl_status_string}
    connection log:
    ${_o2_dl_log}
  ")
  file(WRITE ${_o2_dl_file} "${_o2_dl_log_msg}")
  message(FATAL_ERROR "${_o2_dl_log_title}
      See log: ${_o2_dl_log_file}
  ")
endif()

if(NOT EXISTS ${_o2_dl_file})
  message(FATAL_ERROR "Download file does not exist")
endif()

execute_process(COMMAND ${CMAKE_COMMAND} -E tar xfz ${_o2_dl_file})

# These match variables set by FindO2.cmake
set(O2_INCLUDE_DIR "${_o2_prefix}/src"  CACHE INTERNAL "Path to o2 library headers" FORCE)
set(O2_LIBRARY "" CACHE INTERNAL "Path to o2 built shared library" FORCE)
set(O2_LIBRARY_STATIC "" CACHE INTERNAL "Path to o2 built static library" FORCE)
set(O2_FOUND TRUE CACHE INTERNAL "Whether O2 has been found" FORCE)
