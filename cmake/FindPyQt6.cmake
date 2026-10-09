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

  SET(PYQT6_DIST_INFO CACHE PATH "Path to PyQt6 dist-info directory")
  IF(PYQT6_DIST_INFO)
    SET(_pyqt6_metadata "${PYQT6_DIST_INFO}/METADATA")
  ELSE()
    FILE(GLOB _pyqt6_metadata "${Python_SITEARCH}/PyQt6-*.dist-info/METADATA")
  ENDIF()

  IF(_pyqt6_metadata)
    FILE(READ ${_pyqt6_metadata} _pyqt6_metadata_contents)
    STRING(REGEX REPLACE ".*\nVersion: ([^\n]+).*$" "\\1" PYQT6_VERSION_STR ${_pyqt6_metadata_contents})
  ELSE(_pyqt6_metadata)
    EXECUTE_PROCESS(COMMAND ${Python_EXECUTABLE} -c "from PyQt6.QtCore import PYQT_VERSION_STR; print(PYQT_VERSION_STR, end='')" OUTPUT_VARIABLE PYQT6_VERSION_STR)
  ENDIF(_pyqt6_metadata)

  IF(PYQT6_VERSION_STR)
    EXECUTE_PROCESS(COMMAND ${Python_EXECUTABLE} -c "import os; import PyQt6; print(os.path.dirname(PyQt6.__file__), end='')" OUTPUT_VARIABLE PYQT6_MOD_DIR)
    SET(PYQT6_SIP_DIR "${PYQT6_MOD_DIR}/bindings")
    FIND_PROGRAM(__pyuic6 "pyuic6")
    GET_FILENAME_COMPONENT(PYQT6_BIN_DIR ${__pyuic6} DIRECTORY)

    SET(PYQT6_FOUND TRUE)
  ENDIF(PYQT6_VERSION_STR)

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
