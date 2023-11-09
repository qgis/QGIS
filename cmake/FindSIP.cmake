# Find SIP
# ~~~~~~~~
#
# SIP website: http://www.riverbankcomputing.co.uk/sip/index.php
#
# Find the installed version of SIP. FindSIP should be called after Python
# has been found.
#
# This file defines the following variables:
#
# SIP_VERSION - The version of SIP found expressed as a 6 digit hex number
#     suitable for comparison as a string.
#
# SIP_VERSION_STR - The version of SIP found as a human readable string.
#
# SIP_BINARY_PATH - Path and filename of the SIP command line executable.
#
# SIP_INCLUDE_DIR - Directory holding the SIP C++ header file.
#
# SIP_DEFAULT_SIP_DIR - Default directory where .sip files should be installed
#     into.

# Copyright (c) 2007, Simon Edwards <simon@simonzone.com>
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.



IF(SIP_VERSION)
  # Already in cache, be silent
  SET(SIP_FOUND TRUE)
ELSE(SIP_VERSION)

  FIND_FILE(_find_sip_py FindSIP.py PATHS ${CMAKE_MODULE_PATH} NO_CMAKE_FIND_ROOT_PATH)

  EXECUTE_PROCESS(COMMAND ${Python_EXECUTABLE} ${_find_sip_py} OUTPUT_VARIABLE sip_config)
  IF(sip_config)
    STRING(REGEX REPLACE "^sip_version:([^\n]+).*$" "\\1" SIP_VERSION ${sip_config})
    STRING(REGEX REPLACE ".*\nsip_version_num:([^\n]+).*$" "\\1" SIP_VERSION_NUM ${sip_config})
    STRING(REGEX REPLACE ".*\nsip_version_str:([^\n]+).*$" "\\1" SIP_VERSION_STR ${sip_config})
    STRING(REGEX REPLACE ".*\ndefault_sip_dir:([^\n]+).*$" "\\1" SIP_DEFAULT_SIP_DIR ${sip_config})
    IF(${SIP_VERSION_STR} VERSION_LESS 5)
      STRING(REGEX REPLACE ".*\nsip_bin:([^\n]+).*$" "\\1" SIP_BINARY_PATH ${sip_config})
      STRING(REGEX REPLACE ".*\nsip_inc_dir:([^\n]+).*$" "\\1" SIP_INCLUDE_DIR ${sip_config})
      STRING(REGEX REPLACE ".*\nsip_module_dir:([^\n]+).*$" "\\1" SIP_MODULE_DIR ${sip_config})
    ELSE(${SIP_VERSION_STR} VERSION_LESS 5)
      FIND_PROGRAM(SIP_BUILD_EXECUTABLE sip-build)
    ENDIF(${SIP_VERSION_STR} VERSION_LESS 5)
    SET(SIP_FOUND TRUE)
  ENDIF(sip_config)

  IF(SIP_FOUND)
    IF(NOT SIP_FIND_QUIETLY)
      MESSAGE(STATUS "Found SIP version: ${SIP_VERSION_STR}")
    ENDIF(NOT SIP_FIND_QUIETLY)
  ELSE(SIP_FOUND)
    IF(SIP_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "Could not find SIP")
    ENDIF(SIP_FIND_REQUIRED)
  ENDIF(SIP_FOUND)

ENDIF(SIP_VERSION)

ADD_DEFINITIONS(-DSIP_VERSION=0x${SIP_VERSION})
