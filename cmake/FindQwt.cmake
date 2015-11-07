# Find Qwt
# ~~~~~~~~
# Copyright (c) 2010, Tim Sutton <tim at linfiniti.com>
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#
# Once run this will define:
#
# QWT_FOUND       = system has QWT lib
# QWT_LIBRARY     = full path to the QWT library
# QWT_INCLUDE_DIR = where to find headers
#


FIND_PATH(QWT_INCLUDE_DIR NAMES qwt.h PATHS
  /usr/include
  /usr/local/include
  "$ENV{LIB_DIR}/include"
  "$ENV{INCLUDE}"
  PATH_SUFFIXES qwt-qt4 qwt qwt5 qwt6
  )

if(ENABLE_QT5)
  set(QWT_LIBRARY_NAMES qwt-qt5 qwt6-qt5)
else(ENABLE_QT5)
  set(QWT_LIBRARY_NAMES qwt qwt6 qwt5 qwt-qt4 qwt6-qt4 qwt5-qt4)
endif(ENABLE_QT5)

find_library(QWT_LIBRARY
  NAMES ${QWT_LIBRARY_NAMES}
  PATHS
    /usr/lib
    /usr/local/lib
    "$ENV{LIB_DIR}/lib"
    "$ENV{LIB}"
)

IF (QWT_INCLUDE_DIR AND QWT_LIBRARY)
  SET(QWT_FOUND TRUE)
ENDIF (QWT_INCLUDE_DIR AND QWT_LIBRARY)

IF (QWT_FOUND)
  FILE(READ ${QWT_INCLUDE_DIR}/qwt_global.h qwt_header)
  STRING(REGEX REPLACE "^.*QWT_VERSION_STR +\"([^\"]+)\".*$" "\\1" QWT_VERSION_STR "${qwt_header}")
  IF (NOT QWT_FIND_QUIETLY)
    MESSAGE(STATUS "Found Qwt: ${QWT_LIBRARY} (${QWT_VERSION_STR})")
  ENDIF (NOT QWT_FIND_QUIETLY)
ELSE (QWT_FOUND)
  IF (QWT_FIND_REQUIRED)
    MESSAGE(FATAL_ERROR "Could not find Qwt")
  ENDIF (QWT_FIND_REQUIRED)
ENDIF (QWT_FOUND)
