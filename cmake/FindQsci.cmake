# Find QScintilla2 PyQt module
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#
# QScintilla2 website: http://www.riverbankcomputing.co.uk/software/qscintilla/
#
# Find the installed version of QScintilla2 module. FindQsci should be called
# after Python has been found.
#
# This file defines the following variables:
#
# QSCI_FOUND - system has QScintilla2 PyQt module
#
# QSCI_MOD_VERSION_STR - The version of Qsci module as a human readable string.
#
# Copyright (c) 2012, Larry Shaffer <larrys@dakotacarto.com>
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

IF(QSCI_MOD_VERSION_STR)
  # Already in cache, be silent
  SET(QSCI_FOUND TRUE)
ELSE(QSCI_MOD_VERSION_STR)

  IF(SIP_BUILD_EXECUTABLE)
    # SIP >= 5.0 path

    FILE(GLOB _qsci_metadata "${Python_SITEARCH}/QScintilla*.dist-info/METADATA")
    IF(_qsci_metadata)
      FILE(READ ${_qsci_metadata} _qsci_metadata_contents)
      STRING(REGEX REPLACE ".*\nVersion: ([^\n]+).*$" "\\1" QSCI_MOD_VERSION_STR ${_qsci_metadata_contents})
    ELSE(_qsci_metadata)
      EXECUTE_PROCESS(COMMAND ${Python_EXECUTABLE} -c "from PyQt5.Qsci import QSCINTILLA_VERSION_STR; print(QSCINTILLA_VERSION_STR)" OUTPUT_VARIABLE QSCI_MOD_VERSION_STR)
    ENDIF(_qsci_metadata)

    IF(QSCI_MOD_VERSION_STR)
      SET(QSCI_SIP_DIR "${PYQT5_SIP_DIR}")
      SET(QSCI_FOUND TRUE)
    ENDIF(QSCI_MOD_VERSION_STR)

  ELSE(SIP_BUILD_EXECUTABLE)
    # SIP 4.x path

    FIND_FILE(_find_qsci_py FindQsci.py PATHS ${CMAKE_MODULE_PATH} NO_CMAKE_FIND_ROOT_PATH)

    SET(QSCI_VER 5)

    EXECUTE_PROCESS(COMMAND ${Python_EXECUTABLE} ${_find_qsci_py} ${QSCI_VER} OUTPUT_VARIABLE qsci_ver)

    IF(qsci_ver)
      STRING(REGEX REPLACE "^qsci_version_str:([^\n]+).*$" "\\1" QSCI_MOD_VERSION_STR ${qsci_ver})
      FIND_PATH(QSCI_SIP_DIR
        NAMES Qsci/qscimod5.sip
        PATHS ${PYQT5_SIP_DIR} ${SIP_DEFAULT_SIP_DIR}
      )
      SET(QSCI_FOUND TRUE)
    ENDIF(qsci_ver)

  ENDIF(SIP_BUILD_EXECUTABLE)

  IF(QSCI_FOUND)
    IF(NOT QSCI_FIND_QUIETLY)
      MESSAGE(STATUS "Found QScintilla2 PyQt module: ${QSCI_MOD_VERSION_STR}")
    ENDIF(NOT QSCI_FIND_QUIETLY)
  ELSE(QSCI_FOUND)
    IF(QSCI_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "Could not find QScintilla2 PyQt module")
    ENDIF(QSCI_FIND_REQUIRED)
  ENDIF(QSCI_FOUND)

ENDIF(QSCI_MOD_VERSION_STR)
