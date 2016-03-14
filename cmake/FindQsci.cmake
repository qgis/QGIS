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

IF(EXISTS QSCI_MOD_VERSION_STR)
  # Already in cache, be silent
  SET(QSCI_FOUND TRUE)
ELSE(EXISTS QSCI_MOD_VERSION_STR)

  FIND_FILE(_find_qsci_py FindQsci.py PATHS ${CMAKE_MODULE_PATH})

  IF(ENABLE_QT5)
    SET(QSCI_VER 5)
  ELSE(ENABLE_QT5)
    SET(QSCI_VER 4)
  ENDIF(ENABLE_QT5)

  EXECUTE_PROCESS(COMMAND ${PYTHON_EXECUTABLE} ${_find_qsci_py} ${QSCI_VER} OUTPUT_VARIABLE qsci_ver)

  IF(qsci_ver)
    STRING(REGEX REPLACE "^qsci_version_str:([^\n]+).*$" "\\1" QSCI_MOD_VERSION_STR ${qsci_ver})
    SET(QSCI_FOUND TRUE)
  ENDIF(qsci_ver)

  IF(QSCI_FOUND)
    IF(ENABLE_QT5)
      FIND_PATH(QSCI_SIP_DIR
        NAMES Qsci/qscimod5.sip
        PATHS ${PYQT5_SIP_DIR}
      )
    ELSE(ENABLE_QT5)
      FIND_PATH(QSCI_SIP_DIR
        NAMES Qsci/qscimod4.sip
        PATHS ${PYQT4_SIP_DIR}
      )
    ENDIF(ENABLE_QT5)

    IF(NOT QSCI_FIND_QUIETLY)
      MESSAGE(STATUS "Found QScintilla2 PyQt module: ${QSCI_MOD_VERSION_STR}")
    ENDIF(NOT QSCI_FIND_QUIETLY)
  ELSE(QSCI_FOUND)
    IF(QSCI_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "Could not find QScintilla2 PyQt module")
    ENDIF(QSCI_FIND_REQUIRED)
  ENDIF(QSCI_FOUND)

ENDIF(EXISTS QSCI_MOD_VERSION_STR)
