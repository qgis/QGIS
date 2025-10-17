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


IF(QSCINTILLA_VERSION_STR)
  # Already in cache, be silent
  SET(QSCINTILLA_FOUND TRUE)
ELSE(QSCINTILLA_VERSION_STR)

  if(BUILD_WITH_QT6)
    set(QSCINTILLA_PATH_SUFFIXES qt6)
  else()
    set(QSCINTILLA_PATH_SUFFIXES qt qt5)
  endif()
  
  set(QSCINTILLA_LIBRARY_NAMES
    qscintilla2-${QT_VERSION_BASE_LOWER}
    qscintilla2_${QT_VERSION_BASE_LOWER}
    lib${QT_VERSION_BASE_LOWER}scintilla2
    libqscintilla2-${QT_VERSION_BASE_LOWER}
    ${QT_VERSION_BASE_LOWER}scintilla2
    libqscintilla2-${QT_VERSION_BASE_LOWER}.dylib
    qscintilla2
  )

  find_library(QSCINTILLA_LIBRARY
    NAMES ${QSCINTILLA_LIBRARY_NAMES}
    PATHS
      "${QT_LIBRARY_DIR}"
      $ENV{LIB_DIR}/lib
      /usr/local/lib
      /usr/local/lib/${QT_VERSION_BASE_LOWER}
      /usr/lib
  )

  set(_qsci_fw)
  if(QSCINTILLA_LIBRARY MATCHES "/qscintilla.*\\.framework")
    string(REGEX REPLACE "^(.*/qscintilla.*\\.framework).*$" "\\1" _qsci_fw "${QSCINTILLA_LIBRARY}")
  endif()

  FIND_PATH(QSCINTILLA_INCLUDE_DIR
    NAMES Qsci/qsciglobal.h
    PATHS
      "${_qsci_fw}/Headers"
      ${${QT_VERSION_BASE}Core_INCLUDE_DIRS}
      "${QT_INCLUDE_DIR}"
      $ENV{LIB_DIR}/include
      /usr/local/include
      /usr/include
    PATH_SUFFIXES ${QSCINTILLA_PATH_SUFFIXES}
    )

  IF(QSCINTILLA_LIBRARY AND QSCINTILLA_INCLUDE_DIR)
    SET(QSCINTILLA_FOUND TRUE)

    IF(CYGWIN)
      IF(BUILD_SHARED_LIBS)
      # No need to define QSCINTILLA_USE_DLL here, because it's default for Cygwin.
      ELSE(BUILD_SHARED_LIBS)
        SET (QSCINTILLA_DEFINITIONS -DQSCINTILLA_STATIC)
      ENDIF(BUILD_SHARED_LIBS)
    ENDIF(CYGWIN)
  ENDIF(QSCINTILLA_LIBRARY AND QSCINTILLA_INCLUDE_DIR)

  IF(QSCINTILLA_INCLUDE_DIR AND NOT EXISTS QSCINTILLA_VERSION_STR)
    # get QScintilla2 version from header, is optionally retrieved via bindings
    # with Qsci PyQt4 module
    FILE(READ ${QSCINTILLA_INCLUDE_DIR}/Qsci/qsciglobal.h qsci_header)
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

ENDIF(QSCINTILLA_VERSION_STR)

#MARK_AS_ADVANCED(QSCINTILLA_INCLUDE_DIR QSCINTILLA_LIBRARY)

