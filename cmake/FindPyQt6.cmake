# Find PyQt6
# ~~~~~~~~~~
# Copyright (c) 2007-2008, Simon Edwards <simon@simonzone.com>
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#
# PyQt6 website: http://www.riverbankcomputing.co.uk/pyqt/index.php
#
# Find the installed version of PyQt6. FindPyQt6 should only be called after
# Python has been found.
#
# This file defines the following variables:
#
# PYQT6_VERSION_STR - The version of PyQt6 as a human readable string.
#
# PYQT6_SIP_DIR - The directory holding the PyQt6 .sip files.
#
# PYQT6_SIP_FLAGS - The SIP flags used to build PyQt.

IF(EXISTS PYQT6_VERSION_STR)
  # Already in cache, be silent
  SET(PYQT6_FOUND TRUE)
ELSE(EXISTS PYQT6_VERSION_STR)

  IF(SIP_BUILD_EXECUTABLE)
    # SIP >= 5.0 path

    FILE(GLOB _pyqt6_metadata "${Python_SITEARCH}/PyQt6-*.dist-info/METADATA")
    IF(_pyqt6_metadata)
      FILE(READ ${_pyqt6_metadata} _pyqt6_metadata_contents)
      STRING(REGEX REPLACE ".*\nVersion: ([^\n]+).*$" "\\1" PYQT6_VERSION_STR ${_pyqt6_metadata_contents})
    ELSE(_pyqt6_metadata)
      EXECUTE_PROCESS(COMMAND ${Python_EXECUTABLE} -c "from PyQt6.QtCore import PYQT_VERSION_STR; print(PYQT_VERSION_STR)" OUTPUT_VARIABLE PYQT6_VERSION_STR)
    ENDIF(_pyqt6_metadata)

    IF(PYQT6_VERSION_STR)
      SET(PYQT6_MOD_DIR "${Python_SITEARCH}/PyQt6")
      SET(PYQT6_SIP_DIR "${Python_SITEARCH}/PyQt6/bindings")
      FIND_PROGRAM(__pyuic6 "pyuic6")
      GET_FILENAME_COMPONENT(PYQT6_BIN_DIR ${__pyuic6} DIRECTORY)

      SET(PYQT6_FOUND TRUE)
    ENDIF(PYQT6_VERSION_STR)

  ELSE(SIP_BUILD_EXECUTABLE)
    # SIP 4.x path

    FIND_FILE(_find_pyqt6_py FindPyQt6.py PATHS ${CMAKE_MODULE_PATH} NO_CMAKE_FIND_ROOT_PATH)

    EXECUTE_PROCESS(COMMAND ${Python_EXECUTABLE} ${_find_pyqt6_py} OUTPUT_VARIABLE pyqt_config)
    IF(pyqt_config)
      STRING(REGEX REPLACE "^pyqt_version_str:([^\n]+).*$" "\\1" PYQT6_VERSION_STR ${pyqt_config})
      STRING(REGEX REPLACE ".*\npyqt_mod_dir:([^\n]+).*$" "\\1" PYQT6_MOD_DIR ${pyqt_config})
      STRING(REGEX REPLACE ".*\npyqt_sip_dir:([^\n]+).*$" "\\1" PYQT6_SIP_DIR ${pyqt_config})
      IF(EXISTS ${PYQT6_SIP_DIR}/Qt6)
        SET(PYQT6_SIP_DIR ${PYQT6_SIP_DIR}/Qt6)
      ENDIF(EXISTS ${PYQT6_SIP_DIR}/Qt6)
      STRING(REGEX REPLACE ".*\npyqt_sip_flags:([^\n]+).*$" "\\1" PYQT6_SIP_FLAGS ${pyqt_config})
      STRING(REGEX REPLACE ".*\npyqt_bin_dir:([^\n]+).*$" "\\1" PYQT6_BIN_DIR ${pyqt_config})
      STRING(REGEX REPLACE ".*\npyqt_sip_module:([^\n]+).*$" "\\1" PYQT6_SIP_IMPORT ${pyqt_config})
      SET(PYQT6_FOUND TRUE)
    ENDIF(pyqt_config)

  ENDIF(SIP_BUILD_EXECUTABLE)

  IF(PYQT6_FOUND)
    IF(NOT PyQt6_FIND_QUIETLY)
      MESSAGE(STATUS "Found PyQt6 version: ${PYQT6_VERSION_STR}")
    ENDIF(NOT PyQt6_FIND_QUIETLY)
  ELSE(PYQT6_FOUND)
    IF(PyQt6_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "Could not find PyQt6")
    ENDIF(PyQt6_FIND_REQUIRED)
  ENDIF(PYQT6_FOUND)

ENDIF(EXISTS PYQT6_VERSION_STR)
