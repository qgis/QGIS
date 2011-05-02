# Find QwtPolar
# ~~~~~~~~
# Copyright (c) 2011, Jürgen E. Fischer <jef at norbit.de>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#
# Once run this will define: 
# 
# QWTPOLAR_FOUND       = system has QwtPolar lib
# QWTPOLAR_LIBRARY     = full path to the QwtPolar library
# QWTPOLAR_INCLUDE_DIR = where to find headers 
#


FIND_PATH(QWTPOLAR_INCLUDE_DIR NAMES qwt_polar.h PATHS
  /usr/include
  /usr/local/include
  "$ENV{LIB_DIR}/include" 
  "$ENV{INCLUDE}" 
  PATH_SUFFIXES qwtpolar
  )

FIND_LIBRARY(QWTPOLAR_LIBRARY NAMES qwtpolar PATHS 
  /usr/lib
  /usr/local/lib
  "$ENV{LIB_DIR}/lib" 
  "$ENV{LIB}/lib" 
  )

IF (QWTPOLAR_INCLUDE_DIR AND QWTPOLAR_LIBRARY)
  SET(QWTPOLAR_FOUND TRUE)
ENDIF (QWTPOLAR_INCLUDE_DIR AND QWTPOLAR_LIBRARY)

IF (QWTPOLAR_FOUND)
  IF (NOT QWTPOLAR_FIND_QUIETLY)
    MESSAGE(STATUS "Found QwtPolar: ${QWTPOLAR_LIBRARY}")
  ENDIF (NOT QWTPOLAR_FIND_QUIETLY)
ELSE (QWTPOLAR_FOUND)
  IF (QWTPOLAR_FIND_REQUIRED)
    MESSAGE(FATAL_ERROR "Could not find QwtPolar")
  ENDIF (QWTPOLAR_FIND_REQUIRED)
ENDIF (QWTPOLAR_FOUND)
