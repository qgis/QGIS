# Find Spatialindex
# ~~~~~~~~
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#
# Once run this will define:
#
# SPATIALINDEX_FOUND       = system has Spatialindex lib
# SPATIALINDEX_LIBRARY     = full path to the Spatialindex library
# SPATIALINDEX_INCLUDE_DIR = where to find headers
# SPATIALINDEX_VERSION     = version number


FIND_PATH(SPATIALINDEX_INCLUDE_DIR spatialindex/SpatialIndex.h PATHS
  /usr/include
  /usr/local/include
  "$ENV{LIB_DIR}/include"
  "$ENV{INCLUDE}"
  "$ENV{OSGEO4W_ROOT}/include"
  )

FIND_LIBRARY(SPATIALINDEX_LIBRARY NAMES spatialindex_i spatialindex spatialindex-64 PATHS
  /usr/lib
  /usr/local/lib
  "$ENV{LIB_DIR}/lib"
  "$ENV{LIB}/lib"
  "$ENV{OSGEO4W_ROOT}/lib"
  )

IF (SPATIALINDEX_INCLUDE_DIR AND SPATIALINDEX_LIBRARY)
  SET(SPATIALINDEX_FOUND TRUE)
ENDIF (SPATIALINDEX_INCLUDE_DIR AND SPATIALINDEX_LIBRARY)

IF (SPATIALINDEX_FOUND)
  set(spatialindex_version_file
      "${SPATIALINDEX_INCLUDE_DIR}/spatialindex/Version.h")
  file(STRINGS "${spatialindex_version_file}" spatialindex_version_major REGEX "#define SIDX_VERSION_MAJOR")
  list(GET spatialindex_version_major 0 spatialindex_version_major)
  string(REGEX MATCH "[0-9]+" SPATIALINDEX_VERSION_MAJOR ${spatialindex_version_major} )
  file(STRINGS "${spatialindex_version_file}" spatialindex_version_minor REGEX "#define SIDX_VERSION_MINOR")
  list(GET spatialindex_version_minor 0 spatialindex_version_minor)
  string(REGEX MATCH "[0-9]+" SPATIALINDEX_VERSION_MINOR ${spatialindex_version_minor} )
  file(STRINGS "${spatialindex_version_file}" spatialindex_version_rev REGEX "#define SIDX_VERSION_REV")
  list(GET spatialindex_version_rev 0 spatialindex_version_rev)
  string(REGEX MATCH "[0-9]+" SPATIALINDEX_VERSION_REV ${spatialindex_version_rev} )
  set(SPATIALINDEX_VERSION "${SPATIALINDEX_VERSION_MAJOR}.${SPATIALINDEX_VERSION_MINOR}.${SPATIALINDEX_VERSION_REV}")

  IF (NOT SPATIALINDEX_FIND_QUIETLY)
    MESSAGE(STATUS "Found Spatialindex: ${SPATIALINDEX_LIBRARY} (${SPATIALINDEX_VERSION})")
  ENDIF (NOT SPATIALINDEX_FIND_QUIETLY)
ELSE (SPATIALINDEX_FOUND)
  IF (SPATIALINDEX_FIND_REQUIRED)
    MESSAGE(FATAL_ERROR "Could not find Spatialindex")
  ENDIF (SPATIALINDEX_FIND_REQUIRED)
ENDIF (SPATIALINDEX_FOUND)
