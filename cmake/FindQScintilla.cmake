# Find QScintilla2 PyQt4 module
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#
# QScintilla2 website: http://www.riverbankcomputing.co.uk/software/qscintilla/
#
# Try to find the QScintilla2 includes and library
# which defines
#
# QSCINTILLA_FOUND - system has QScintilla2
# QSCINTILLA_INCLUDE_DIR - where to find qextscintilla.h
# QSCINTILLA_LIBRARY - where to find the QScintilla2 library
# QSCINTILLA_VERSION_STR - version of library

# copyright (c) 2007 Thomas Moenicke thomas.moenicke@kdemail.net
#
# Redistribution and use is allowed according to the terms of the FreeBSD license.

# Edited by Larry Shaffer, 2012
# NOTE: include after check for Qt


IF(EXISTS QSCINTILLA_VERSION_STR)
  # Already in cache, be silent
  SET(QSCINTILLA_FOUND TRUE)
ELSE(EXISTS QSCINTILLA_VERSION_STR)

  FIND_PATH(QSCINTILLA_INCLUDE_DIR
    NAMES qsciglobal.h
    PATHS
      "${QT_INCLUDE_DIR}/Qsci"
      /usr/local/include/Qsci
      /usr/include/Qsci
      /usr/include
    )

  FIND_LIBRARY(QSCINTILLA_LIBRARY
    NAMES qscintilla2 libqscintilla2 libqscintilla2.dylib
    PATHS
      "${QT_LIBRARY_DIR}"
      /usr/local/lib
      /usr/lib
    )

  IF(QSCINTILLA_LIBRARY)
    # QSCINTILLA_INCLUDE_DIR is not required at this time (Oct 2012) since only
    # Qsci PyQt4 module is used, though lib is needed for Mac bundling
    SET(QSCINTILLA_FOUND TRUE)

    IF(CYGWIN)
      IF(BUILD_SHARED_LIBS)
      # No need to define QSCINTILLA_USE_DLL here, because it's default for Cygwin.
      ELSE(BUILD_SHARED_LIBS)
        SET (QSCINTILLA_DEFINITIONS -DQSCINTILLA_STATIC)
      ENDIF(BUILD_SHARED_LIBS)
    ENDIF(CYGWIN)
  ENDIF(QSCINTILLA_LIBRARY)

  IF(QSCINTILLA_INCLUDE_DIR AND NOT EXISTS QSCINTILLA_VERSION_STR)
    # get QScintilla2 version from header, is optinally retrieved via bindings
    # with Qsci PyQt4 module
    FILE(READ ${QSCINTILLA_INCLUDE_DIR}/qsciglobal.h qsci_header)
    STRING(REGEX REPLACE "^.*QSCINTILLA_VERSION_STR +\"([^\"]+)\".*$" "\\1" QSCINTILLA_VERSION_STR "${qsci_header}")
  ENDIF(QSCINTILLA_INCLUDE_DIR AND NOT EXISTS QSCINTILLA_VERSION_STR)

  IF(QSCINTILLA_FOUND)
    IF(NOT QSCINTILLA_FIND_QUIETLY)
      MESSAGE(STATUS "Found QScintilla2: ${QSCINTILLA_LIBRARY} (${QSCINTILLA_VERSION_STR})")
    ENDIF(NOT QSCINTILLA_FIND_QUIETLY)
  ELSE(QSCINTILLA_FOUND)
    IF(QSCINTILLA_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "Could not find QScintilla2 library")
    ENDIF(QSCINTILLA_FIND_REQUIRED)
  ENDIF(QSCINTILLA_FOUND)

ENDIF(EXISTS QSCINTILLA_VERSION_STR)

#MARK_AS_ADVANCED(QSCINTILLA_INCLUDE_DIR QSCINTILLA_LIBRARY)

