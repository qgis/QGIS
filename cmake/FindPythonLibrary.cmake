# Find Python
# ~~~~~~~~~~~
# Find the Python interpreter and related Python directories.
#
# This file defines the following variables:
#
# PYTHON_EXECUTABLE - The path and filename of the Python interpreter.
#
# PYTHON_SHORT_VERSION - The version of the Python interpreter found,
#     excluding the patch version number. (e.g. 2.5 and not 2.5.1))
#
# PYTHON_LONG_VERSION - The version of the Python interpreter found as a human
#     readable string.
#
# PYTHON_SITE_PACKAGES_DIR - Location of the Python site-packages directory.
#
# PYTHON_INCLUDE_PATH - Directory holding the python.h include file.
#
# PYTHON_LIBRARY, PYTHON_LIBRARIES- Location of the Python library.

# Copyright (c) 2007, Simon Edwards <simon@simonzone.com>
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.



INCLUDE(CMakeFindFrameworks)

if(EXISTS "${PYTHON_INCLUDE_PATH}" AND EXISTS "${PYTHON_LIBRARY}" AND EXISTS "${PYTHON_SITE_PACKAGES_DIR}")
  # Already in cache, be silent
  set(PYTHONLIBRARY_FOUND TRUE)
else(EXISTS "${PYTHON_INCLUDE_PATH}" AND EXISTS "${PYTHON_LIBRARY}" AND EXISTS "${PYTHON_SITE_PACKAGES_DIR}")

  FIND_PACKAGE(PythonInterp 3)

  if(PYTHONINTERP_FOUND)
    FIND_FILE(_find_lib_python_py FindLibPython.py PATHS ${CMAKE_MODULE_PATH} NO_CMAKE_FIND_ROOT_PATH)

    EXECUTE_PROCESS(COMMAND ${PYTHON_EXECUTABLE}  ${_find_lib_python_py} OUTPUT_VARIABLE python_config)
    if(python_config)
      STRING(REGEX REPLACE ".*exec_prefix:([^\n]+).*$" "\\1" PYTHON_PREFIX ${python_config})
      STRING(REGEX REPLACE ".*\nshort_version:([^\n]+).*$" "\\1" PYTHON_SHORT_VERSION ${python_config})
      STRING(REGEX REPLACE ".*\nlong_version:([^\n]+).*$" "\\1" PYTHON_LONG_VERSION ${python_config})
      STRING(REGEX REPLACE ".*\npy_inc_dir:([^\n]+).*$" "\\1" PYTHON_INCLUDE_PATH ${python_config})
      if(NOT PYTHON_SITE_PACKAGES_DIR)
        if(NOT PYTHON_LIBS_WITH_KDE_LIBS)
          STRING(REGEX REPLACE ".*\nsite_packages_dir:([^\n]+).*$" "\\1" PYTHON_SITE_PACKAGES_DIR ${python_config})
        else(NOT PYTHON_LIBS_WITH_KDE_LIBS)
          set(PYTHON_SITE_PACKAGES_DIR ${KDE4_LIB_INSTALL_DIR}/python${PYTHON_SHORT_VERSION}/site-packages)
        endif(NOT PYTHON_LIBS_WITH_KDE_LIBS)
      endif(NOT PYTHON_SITE_PACKAGES_DIR)
      STRING(REGEX REPLACE "([0-9]+).([0-9]+)" "\\1\\2" PYTHON_SHORT_VERSION_NO_DOT ${PYTHON_SHORT_VERSION})
      set(PYTHON_LIBRARY_NAMES python${PYTHON_SHORT_VERSION} python${PYTHON_SHORT_VERSION_NO_DOT} python${PYTHON_SHORT_VERSION}m python${PYTHON_SHORT_VERSION_NO_DOT}m)
      if(WIN32)
          STRING(REPLACE "\\" "/" PYTHON_SITE_PACKAGES_DIR ${PYTHON_SITE_PACKAGES_DIR})
          FIND_LIBRARY(PYTHON_LIBRARY NAMES ${PYTHON_LIBRARY_NAMES} PATHS ${PYTHON_PREFIX}/lib ${PYTHON_PREFIX}/libs)
      endif(WIN32)
      FIND_LIBRARY(PYTHON_LIBRARY NAMES ${PYTHON_LIBRARY_NAMES})
      set(PYTHON_INCLUDE_PATH ${PYTHON_INCLUDE_PATH} CACHE FILEPATH "Directory holding the python.h include file" FORCE)
      set(PYTHONLIBRARY_FOUND TRUE)
    endif(python_config)

    # adapted from cmake's builtin FindPythonLibs
    if(APPLE)
        # If a framework has been detected in the include path, make sure
        # framework's versioned library (not any .dylib) is used for linking
        # NOTE: don't rely upon Python.framework/Versions/Current, since that may be 2.7
        if("${PYTHON_INCLUDE_PATH}" MATCHES "Python\\.framework")
          set(PYTHON_LIBRARY "")
          set(PYTHON_DEBUG_LIBRARY "")
          # get clean path to just framework
          STRING(REGEX REPLACE "^(.*/Python\\.framework).*$" "\\1" _py_fw "${PYTHON_INCLUDE_PATH}")
          if("${_py_fw}" MATCHES "Cellar/python")
            # Appears to be a Homebrew Python install; do specific fix ups
            # get Homebrew prefix (may not be /usr/local)
            STRING(REGEX REPLACE "^(.+)/Cellar.*$" "\\1" _hb_prefix "${_py_fw}")
            # prefer the Homebrew prefix framework over only versioned Python keg
            set(_py_fw "${_hb_prefix}/Frameworks/Python.framework")
            # prefer the symlinked-to Homebrew site-packages over only versioned Python keg
            set(PYTHON_SITE_PACKAGES_DIR "${_hb_prefix}/lib/python${PYTHON_SHORT_VERSION}/site-packages")
          endif("${_py_fw}" MATCHES "Cellar/python")
          # prefer the Headers subdirectory for includes
          if(EXISTS "${_py_fw}/Versions/${PYTHON_SHORT_VERSION}/Headers")
            set(PYTHON_INCLUDE_PATH "${_py_fw}/Versions/${PYTHON_SHORT_VERSION}/Headers" CACHE FILEPATH "Directory holding the python.h include file" FORCE)
          endif(EXISTS "${_py_fw}/Versions/${PYTHON_SHORT_VERSION}/Headers")
        endif("${PYTHON_INCLUDE_PATH}" MATCHES "Python\\.framework")
        if(NOT PYTHON_LIBRARY)
          # ensure the versioned framework's library is defined, instead of relying upon -F search paths
          if(EXISTS "${_py_fw}/Versions/${PYTHON_SHORT_VERSION}/Python")
            set(PYTHON_LIBRARY "${_py_fw}/Versions/${PYTHON_SHORT_VERSION}/Python" CACHE FILEPATH "Python framework library" FORCE)
          endif(EXISTS "${_py_fw}/Versions/${PYTHON_SHORT_VERSION}/Python")
        endif(NOT PYTHON_LIBRARY)
        if(PYTHON_LIBRARY)
          set(PYTHONLIBRARY_FOUND TRUE)
        endif(PYTHON_LIBRARY)
    endif(APPLE)
  endif(PYTHONINTERP_FOUND)

  if(PYTHONLIBRARY_FOUND)
    if(APPLE)
      # keep reference to system or custom python site-packages
      # useful during app-bundling operations
      set(PYTHON_SITE_PACKAGES_SYS ${PYTHON_SITE_PACKAGES_DIR})
    endif(APPLE)
    set(PYTHON_LIBRARIES ${PYTHON_LIBRARY})
    if(NOT PYTHONLIBRARY_FIND_QUIETLY)
      message(STATUS "Found Python executable: ${PYTHON_EXECUTABLE}")
      message(STATUS "Found Python version: ${PYTHON_LONG_VERSION}")
      message(STATUS "Found Python library: ${PYTHON_LIBRARY}")
      message(STATUS "Found Python site-packages: ${PYTHON_SITE_PACKAGES_DIR}")
    endif(NOT PYTHONLIBRARY_FIND_QUIETLY)
  else(PYTHONLIBRARY_FOUND)
    if(PYTHONLIBRARY_FIND_REQUIRED)
      message(FATAL_ERROR "Could not find Python")
    endif(PYTHONLIBRARY_FIND_REQUIRED)
  endif(PYTHONLIBRARY_FOUND)

endif (EXISTS "${PYTHON_INCLUDE_PATH}" AND EXISTS "${PYTHON_LIBRARY}" AND EXISTS "${PYTHON_SITE_PACKAGES_DIR}")
