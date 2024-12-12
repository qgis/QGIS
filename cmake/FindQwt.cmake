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

set(QWT_LIB_SUFFIX "")
if (MSVC AND QGISDEBUG)
  set(QWT_LIB_SUFFIX "d")
endif()
set(QWT_LIBRARY_NAMES qwt-${QT_VERSION_BASE_LOWER}${QWT_LIB_SUFFIX}
  qwt${QWT_LIB_SUFFIX} qwt6${QWT_LIB_SUFFIX} qwt6-${QT_VERSION_BASE_LOWER}${QWT_LIB_SUFFIX})

find_library(QWT_LIBRARY
  NAMES ${QWT_LIBRARY_NAMES}
  PATHS
    /usr/lib
    /usr/lib64
    /usr/local/lib
    /usr/local/lib/${QT_VERSION_BASE_LOWER}
    "$ENV{LIB_DIR}/lib"
    "$ENV{LIB}"
)

set(_qwt_fw)
if(QWT_LIBRARY MATCHES "/qwt.*\\.framework")
  string(REGEX REPLACE "^(.*/qwt.*\\.framework).*$" "\\1" _qwt_fw "${QWT_LIBRARY}")
endif()

FIND_PATH(QWT_INCLUDE_DIR NAMES qwt.h PATHS
  "${_qwt_fw}/Headers"
  /usr/include
  /usr/include/${QT_VERSION_BASE_LOWER}
  /usr/local/include
  /usr/local/include/${QT_VERSION_BASE_LOWER}
  "$ENV{LIB_DIR}/include"
  "$ENV{INCLUDE}"
  PATH_SUFFIXES qwt-${QT_VERSION_BASE_LOWER} ${QT_VERSION_BASE_LOWER}/qwt qwt qwt6
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
