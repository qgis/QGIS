# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.
# based on https://github.com/hlrs-vis/covise/blob/b909e6fad418e23e48726ab44de1bb36e82a24f8/cmake/FindDRACO.cmake#L4

# - Find DRACO
# Find the DRACO includes and library
#
#  DRACO_INCLUDE_DIR - Where to find DRACO includes
#  DRACO_FOUND       - True if DRACO was found

IF(DRACO_INCLUDE_DIR)
  SET(DRACO_FIND_QUIETLY TRUE)
ENDIF(DRACO_INCLUDE_DIR)

FIND_PATH(DRACO_INCLUDE_DIR "draco/draco_features.h"
  PATHS
  $ENV{DRACO_HOME}/include
  $ENV{EXTERNLIBS}/DRACO/include
  ~/Library/Frameworks
  /Library/Frameworks
  /usr/local
  /usr
  /sw # Fink
  /opt/local # DarwinPorts
  /opt/csw # Blastwave
  /opt
  DOC "DRACO - Headers"
)

SET(DRACO_NAMES draco)
SET(DRACO_DBG_NAMES dracod)

FIND_LIBRARY(DRACO_LIBRARY NAMES ${DRACO_NAMES}
  PATHS
  $ENV{DRACO_HOME}
  $ENV{EXTERNLIBS}/DRACO
  ~/Library/Frameworks
  /Library/Frameworks
  /usr/local
  /usr
  /sw
  /opt/local
  /opt/csw
  /opt
  PATH_SUFFIXES lib lib64
  DOC "DRACO - Library"
)

INCLUDE(FindPackageHandleStandardArgs)

SET(DRACO_LIBRARIES ${DRACO_LIBRARY})

FIND_PACKAGE_HANDLE_STANDARD_ARGS(Draco DEFAULT_MSG DRACO_LIBRARY DRACO_INCLUDE_DIR)

MARK_AS_ADVANCED(DRACO_LIBRARY DRACO_INCLUDE_DIR)

IF(DRACO_FOUND)
  SET(DRACO_INCLUDE_DIRS ${DRACO_INCLUDE_DIR})

  set(draco_version_file
        "${DRACO_INCLUDE_DIR}/draco/core/draco_version.h")
    file(STRINGS "${draco_version_file}" draco_version REGEX "kDracoVersion")
    list(GET draco_version 0 draco_version)
    string(REPLACE "static const char kDracoVersion[] = " "" draco_version
                   "${draco_version}")
    string(REPLACE ";" "" draco_version "${draco_version}")
    string(REPLACE "\"" "" draco_version "${draco_version}")
    set(DRACO_VERSION ${draco_version})

ENDIF(DRACO_FOUND)
