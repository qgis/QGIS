# ~~~~~~~~~~
# Copyright (c) 2012, Juergen E. Fischer <jef at norbit dot de>
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#
# CMake module to search for OCI library
#
# If it's found it sets OCI_FOUND to TRUE
# and following variables are set:
#    OCI_INCLUDE_DIR
#    OCI_LIBRARY

FIND_PATH(OCI_INCLUDE_DIR oci.h
  PATHS
  ${ORACLE_INCLUDEDIR}
  $ENV{OSGEO4W_ROOT}/include
  $ENV{ORACLE_HOME}/rdbms/public
)

FIND_LIBRARY(OCI_LIBRARY clntsh oci
  PATHS
  ${ORACLE_LIBDIR}
  $ENV{OSGEO4W_ROOT}/lib
  $ENV{ORACLE_HOME}/lib
)

IF (OCI_INCLUDE_DIR)
  SET(OCI_FOUND TRUE)
ELSE (OCI_INCLUDE_DIR)
  SET(OCI_FOUND FALSE)
ENDIF(OCI_INCLUDE_DIR)

IF (OCI_FOUND)
 IF (NOT OCI_FIND_QUIETLY)
    MESSAGE(STATUS "Found OCI: ${OCI_LIBRARY}")
 ENDIF (NOT OCI_FIND_QUIETLY)
ELSE (OCI_FOUND)
   IF (OCI_FIND_REQUIRED)
     MESSAGE(FATAL_ERROR "Could not find OCI")
   ELSE (OCI_FIND_REQUIRED)
     IF (NOT OCI_FIND_QUIETLY)
        MESSAGE(STATUS "Could not find OCI")
     ENDIF (NOT OCI_FIND_QUIETLY)
   ENDIF (OCI_FIND_REQUIRED)
ENDIF (OCI_FOUND)
