# OpenThreads is a C++ based threading library. Its largest userbase
# seems to OpenSceneGraph so you might notice I accept OSGDIR as an
# environment path.
# I consider this part of the Findosg* suite used to find OpenSceneGraph
# components.
# Each component is separate and you must opt in to each module.
#
# Locate OpenThreads
# This module defines
# OPENTHREADS_LIBRARY
# OPENTHREADS_FOUND, if false, do not try to link to OpenThreads
# OPENTHREADS_INCLUDE_DIR, where to find the headers
#
# $OPENTHREADS_DIR is an environment variable that would
# correspond to the ./configure --prefix=$OPENTHREADS_DIR
# used in building osg.
#
# Created by Eric Wing.

# Header files are presumed to be included like
# #include <OpenThreads/Thread>

# To make it easier for one-step automated configuration/builds,
# we leverage environmental paths. This is preferable
# to the -DVAR=value switches because it insulates the
# users from changes we may make in this script.
# It also offers a little more flexibility than setting
# the CMAKE_*_PATH since we can target specific components.
# However, the default CMake behavior will search system paths
# before anything else. This is problematic in the cases
# where you have an older (stable) version installed, but
# are trying to build a newer version.
# CMake doesn't offer a nice way to globally control this behavior
# so we have to do a nasty "double FIND_" in this module.
# The first FIND disables the CMAKE_ search paths and only checks
# the environmental paths.
# If nothing is found, then the second find will search the
# standard install paths.
# Explicit -DVAR=value arguments should still be able to override everything.
# Note: We have added an additional check for ${CMAKE_PREFIX_PATH}.
# This is not an official CMake variable, but one we are proposing be
# added to CMake. Be warned that this may go away or the variable name
# may change.

FIND_PATH(OPENTHREADS_INCLUDE_DIR OpenThreads/Thread
    $ENV{OPENTHREADS_INCLUDE_DIR}
    $ENV{OPENTHREADS_DIR}/include
    $ENV{OPENTHREADS_DIR}
    $ENV{OSG_INCLUDE_DIR}
    $ENV{OSG_DIR}/include
    $ENV{OSG_DIR}
    NO_DEFAULT_PATH
)

IF(NOT OPENTHREADS_INCLUDE_DIR)
    FIND_PATH(OPENTHREADS_INCLUDE_DIR OpenThreads/Thread
        PATHS ${CMAKE_PREFIX_PATH} # Unofficial: We are proposing this.
        PATH_SUFFIXES include
    )
ENDIF(NOT OPENTHREADS_INCLUDE_DIR)

IF(NOT OPENTHREADS_INCLUDE_DIR)
    FIND_PATH(OPENTHREADS_INCLUDE_DIR OpenThreads/Thread
        ~/Library/Frameworks
        /Library/Frameworks
        /usr/local/include
        /usr/include
        /sw/include # Fink
        /opt/local/include # DarwinPorts
        /opt/csw/include # Blastwave
        /opt/include
        [HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Session\ Manager\\Environment;OpenThreads_ROOT]/include
        [HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Session\ Manager\\Environment;OSG_ROOT]/include
    )
ENDIF(NOT OPENTHREADS_INCLUDE_DIR)


FIND_LIBRARY(OPENTHREADS_LIBRARY
    NAMES OpenThreads OpenThreadsWin32
    PATHS
    $ENV{OPENTHREADS_LIBRARY_DIR}
    $ENV{OPENTHREADS_DIR}/lib64
    $ENV{OPENTHREADS_DIR}/lib
    $ENV{OPENTHREADS_DIR}
    $ENV{OSG_LIBRARY_DIR}
    $ENV{OSG_DIR}/lib64
    $ENV{OSG_DIR}/lib
    $ENV{OSG_DIR}
    NO_DEFAULT_PATH
)

IF(NOT OPENTHREADS_LIBRARY)
    FIND_LIBRARY(OPENTHREADS_LIBRARY
        NAMES OpenThreads OpenThreadsWin32
        PATHS ${CMAKE_PREFIX_PATH} # Unofficial: We are proposing this.
        PATH_SUFFIXES lib64 lib
    )
ENDIF(NOT OPENTHREADS_LIBRARY)

IF(NOT OPENTHREADS_LIBRARY)
    FIND_LIBRARY(OPENTHREADS_LIBRARY
        NAMES OpenThreads OpenThreadsWin32
        PATHS
        ~/Library/Frameworks
        /Library/Frameworks
        /usr/local/lib64
        /usr/local/lib
        /usr/lib64
        /usr/lib
        /sw/lib64
        /sw/lib
        /opt/local/lib64
        /opt/local/lib
        /opt/csw/lib64
        /opt/csw/lib
        /opt/lib64
        /opt/lib
        [HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Session\ Manager\\Environment;OpenThreads_ROOT]/lib
        [HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Session\ Manager\\Environment;OSG_ROOT]/lib
    )
ENDIF(NOT OPENTHREADS_LIBRARY)


FIND_LIBRARY(OPENTHREADS_LIBRARY_DEBUG
    NAMES OpenThreadsd OpenThreadsWin32d
    PATHS
    $ENV{OPENTHREADS_DEBUG_LIBRARY_DIR}
    $ENV{OPENTHREADS_LIBRARY_DIR}
    $ENV{OPENTHREADS_DIR}/lib64
    $ENV{OPENTHREADS_DIR}/lib
    $ENV{OPENTHREADS_DIR}
    $ENV{OSG_LIBRARY_DIR}
    $ENV{OSG_DIR}/lib64
    $ENV{OSG_DIR}/lib
    $ENV{OSG_DIR}
    ${CMAKE_PREFIX_PATH}/lib64
    ${CMAKE_PREFIX_PATH}/lib
    ${CMAKE_PREFIX_PATH}
    NO_DEFAULT_PATH
)

IF(NOT OPENTHREADS_LIBRARY_DEBUG)
    FIND_LIBRARY(OPENTHREADS_LIBRARY_DEBUG
        NAMES OpenThreadsd OpenThreadsWin32d
        PATHS ${CMAKE_PREFIX_PATH} # Unofficial: We are proposing this.
        PATH_SUFFIXES lib64 lib
    )
ENDIF(NOT OPENTHREADS_LIBRARY_DEBUG)

IF(NOT OPENTHREADS_LIBRARY_DEBUG)
    FIND_LIBRARY(OPENTHREADS_LIBRARY_DEBUG
        NAMES OpenThreadsd OpenThreadsWin32d
        PATHS
        /usr/local/lib64
        /usr/local/lib
        /usr/lib64
        /usr/lib
        /sw/lib64
        /sw/lib
        /opt/local/lib64
        /opt/local/lib
        /opt/csw/lib64
        /opt/csw/lib
        /opt/lib64
        /opt/lib
        [HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Session\ Manager\\Environment;OpenThreads_ROOT]/lib
        [HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Session\ Manager\\Environment;OSG_ROOT]/lib
    )
ENDIF(NOT OPENTHREADS_LIBRARY_DEBUG)


IF(OPENTHREADS_LIBRARY)
  IF(NOT OPENTHREADS_LIBRARY_DEBUG)
      #MESSAGE("-- Warning Debug OpenThreads not found, using: ${OPENTHREADS_LIBRARY}")
      #SET(OPENTHREADS_LIBRARY_DEBUG "${OPENTHREADS_LIBRARY}")
      SET(OPENTHREADS_LIBRARY_DEBUG "${OPENTHREADS_LIBRARY}" CACHE FILEPATH "Debug version of OpenThreads Library (use regular version if not available)" FORCE)
  ENDIF(NOT OPENTHREADS_LIBRARY_DEBUG)
ENDIF(OPENTHREADS_LIBRARY)

SET(OPENTHREADS_FOUND "NO")
IF(OPENTHREADS_INCLUDE_DIR AND OPENTHREADS_LIBRARY)
  SET(OPENTHREADS_FOUND "YES")
  # MESSAGE("-- Found OpenThreads: "${OPENTHREADS_LIBRARY})
ENDIF(OPENTHREADS_INCLUDE_DIR AND OPENTHREADS_LIBRARY)

