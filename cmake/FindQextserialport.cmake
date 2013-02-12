# Find Qextserialport
# ~~~~~~~~
# Copyright (c) 2011, Jürgen E. Fischer <jef at norbit.de>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#
# Once run this will define:
#
# QEXTSERIALPORT_FOUND       = system has Qextserialport lib
# QEXTSERIALPORT_LIBRARY     = full path to the Qextserialport library
# QEXTSERIALPORT_INCLUDE_DIR = where to find headers
#


FIND_PATH(QEXTSERIALPORT_INCLUDE_DIR NAMES qextserialport.h PATHS
  /usr/include
  /usr/local/include
  "$ENV{LIB_DIR}/include"
  "$ENV{INCLUDE}"
  PATH_SUFFIXES QtExtSerialPort
  )

FIND_LIBRARY(QEXTSERIALPORT_LIBRARY NAMES qextserialport-1.2 PATHS
  /usr/lib
  /usr/local/lib
  "$ENV{LIB_DIR}/lib"
  "$ENV{LIB}/lib"
  )

IF (QEXTSERIALPORT_INCLUDE_DIR AND QEXTSERIALPORT_LIBRARY)
  SET(QEXTSERIALPORT_FOUND TRUE)
ENDIF (QEXTSERIALPORT_INCLUDE_DIR AND QEXTSERIALPORT_LIBRARY)

IF (QEXTSERIALPORT_FOUND)
  IF (NOT QEXTSERIALPORT_FIND_QUIETLY)
    MESSAGE(STATUS "Found Qextserialport: ${QEXTSERIALPORT_LIBRARY}")
  ENDIF (NOT QEXTSERIALPORT_FIND_QUIETLY)
ELSE (QEXTSERIALPORT_FOUND)
  IF (QEXTSERIALPORT_FIND_REQUIRED)
    MESSAGE(FATAL_ERROR "Could not find Qextserialport")
  ENDIF (QEXTSERIALPORT_FIND_REQUIRED)
ENDIF (QEXTSERIALPORT_FOUND)
