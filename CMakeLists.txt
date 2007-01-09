
PROJECT(qgis)

# TODO:
# - install includes for libs
# - nice output when configured
# - rename *.ui files to have the same filename as their implementation
#   e.g. instead of blahblahbase.ui use blahblah.ui
#   because it's more common in Qt4

#############################################################
# CMake settings

CMAKE_MINIMUM_REQUIRED(VERSION 2.4.3)

SET(CMAKE_COLOR_MAKEFILE ON)

# set path to additional CMake modules
SET(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

# it's possible to set PLUGINS_ALSO_BINARIES to TRUE
# then some plugins that can run as standalone apps will be built
# also as standalone apps
SET (PLUGINS_ALSO_BINARIES FALSE)


#############################################################
# check if lexer and parser are not missing
# http://www.mail-archive.com/cmake@cmake.org/msg02861.html

INCLUDE(Flex)

FIND_FLEX()

IF (NOT FLEX_EXECUTABLE)
  MESSAGE(FATAL_ERROR "Couldn't find Flex")
ENDIF (NOT FLEX_EXECUTABLE)

INCLUDE(Bison)

FIND_BISON()

IF (NOT BISON_EXECUTABLE)
  MESSAGE(FATAL_ERROR "Couldn't find Bison")
ENDIF (NOT BISON_EXECUTABLE)

#############################################################
# search for dependencies

# required
FIND_PACKAGE(Proj)
FIND_PACKAGE(Sqlite3)
FIND_PACKAGE(GEOS)
FIND_PACKAGE(GDAL)

# optional
FIND_PACKAGE(Postgres) # PostgreSQL provider, SPIT plugin
FIND_PACKAGE(Expat)    # GPS importer plugin
FIND_PACKAGE(GSL)      # Georeferencer
FIND_PACKAGE(GRASS)    # GRASS plugin
FIND_PACKAGE(PythonLibs) # MapServer export tool
FIND_PACKAGE(PythonInterp) # test for sip and PyQt4

IF (NOT PROJ_FOUND OR NOT SQLITE3_FOUND OR NOT GEOS_FOUND OR NOT GDAL_FOUND)
  MESSAGE (SEND_ERROR "Some dependencies were not found!")
ENDIF (NOT PROJ_FOUND OR NOT SQLITE3_FOUND OR NOT GEOS_FOUND OR NOT GDAL_FOUND)

IF (POSTGRES_FOUND)
  # following variable is used in qgsconfig.h
  SET (HAVE_POSTGRESQL TRUE)
ENDIF (POSTGRES_FOUND)


#############################################################
# python support

MACRO (TRY_RUN_PYTHON RESULT CMD)
  IF (PYTHONINTERP_FOUND)
    
    EXEC_PROGRAM(${PYTHON_EXECUTABLE} ARGS -c "\"${CMD}\""
                 OUTPUT_VARIABLE out
                 RETURN_VALUE retval)
    IF (retval EQUAL 0)
      SET (${RESULT} TRUE)
    ELSE (retval EQUAL 0)
      SET (${RESULT} FALSE)
    ENDIF (retval EQUAL 0)
  
  ELSE (PYTHONINTERP_FOUND)
    SET (${${RESULT}} FALSE)
  ENDIF (PYTHONINTERP_FOUND)
ENDMACRO (TRY_RUN_PYTHON)


# enable/disable python support (mapserver export tool and bindings)
IF (PYTHON_LIBRARIES AND PYTHON_INCLUDE_PATH)
  SET (PYTHON_FOUND TRUE)
  MESSAGE(STATUS "Python libraries found")

  # TODO: should not be needed, report it to CMake devs
  IF (UNIX AND NOT APPLE)
    SET (PYTHON_LIBRARIES ${PYTHON_LIBRARIES} util)
  ENDIF (UNIX AND NOT APPLE)
  
  # check for SIP
  TRY_RUN_PYTHON (HAVE_SIP_MODULE "from sip import wrapinstance")
  FIND_PROGRAM (SIP_BINARY_PATH sip)

  # check for PyQt4
  TRY_RUN_PYTHON (HAVE_PYQT4 "from PyQt4 import QtCore, QtGui")
  
  # if SIP and PyQt4 are found, enable bindings
  IF (HAVE_SIP_MODULE AND SIP_BINARY_PATH AND HAVE_PYQT4)
    SET (HAVE_PYTHON TRUE)
    MESSAGE(STATUS "Python bindings enabled")
  ELSE (HAVE_SIP_MODULE AND SIP_BINARY_PATH AND HAVE_PYQT4)
    SET (HAVE_PYTHON FALSE)
    MESSAGE(STATUS "Python bindings disabled")
  ENDIF (HAVE_SIP_MODULE AND SIP_BINARY_PATH AND HAVE_PYQT4)

ENDIF (PYTHON_LIBRARIES AND PYTHON_INCLUDE_PATH)



#############################################################
# search for Qt4

SET( QT_USE_QT3SUPPORT TRUE )
SET( QT_USE_QTXML      TRUE )
SET( QT_USE_QTNETWORK  TRUE )
SET( QT_USE_QTSVG      TRUE )
# TODO: should not be needed, report it to CMake devs
IF (APPLE AND QT_USE_QT3SUPPORT)
  SET( QT_USE_QTSQL    TRUE )
ENDIF (APPLE AND QT_USE_QT3SUPPORT)

FIND_PACKAGE(Qt4 REQUIRED)

INCLUDE( ${QT_USE_FILE} )

FIND_PROGRAM(QT_LRELEASE_EXECUTABLE
    NAMES lrelease
    PATHS ${QT_BINARY_DIR}
    NO_DEFAULT_PATH
    )

#############################################################
# enable warnings

ADD_DEFINITIONS( -Wall )

IF (CMAKE_BUILD_TYPE MATCHES Debug)
  ADD_DEFINITIONS(-DQGISDEBUG=1)
ENDIF (CMAKE_BUILD_TYPE MATCHES Debug)

#############################################################
# platform specific stuff

IF (WIN32)

  SET (QGIS_BIN_DIR    ${CMAKE_INSTALL_PREFIX})
  SET (QGIS_DATA_DIR   ${CMAKE_INSTALL_PREFIX})
  SET (QGIS_PLUGIN_DIR ${CMAKE_INSTALL_PREFIX}/plugins)

ELSE (WIN32)

  # common for MAC and UNIX
  SET (QGIS_BIN_DIR    ${CMAKE_INSTALL_PREFIX}/bin)
  SET (QGIS_DATA_DIR   ${CMAKE_INSTALL_PREFIX}/share/qgis)
  SET (QGIS_PLUGIN_DIR ${CMAKE_INSTALL_PREFIX}/lib/qgis)

  IF (UNIX AND NOT APPLE)
    ADD_DEFINITIONS(-DPREFIX=\\"${CMAKE_INSTALL_PREFIX}\\")
    ADD_DEFINITIONS(-DPLUGINPATH=\\"${QGIS_PLUGIN_DIR}\\")
    ADD_DEFINITIONS(-DPKGDATAPATH=\\"${QGIS_DATA_DIR}\\")
  ENDIF (UNIX AND NOT APPLE)

ENDIF (WIN32)


IF (WIN32)
  # expect that classes are being imported
  # Note: MSVC doesn't like when the macros are quotes
  # and MSYS doesn't like them unqouted (bacause of braces)
  IF (MSVC)
    ADD_DEFINITIONS("-DCORE_EXPORT=__declspec(dllimport)")
    ADD_DEFINITIONS("-DGUI_EXPORT=__declspec(dllimport)")
  ELSE (MSVC)
    ADD_DEFINITIONS("\"-DCORE_EXPORT=__declspec(dllimport)\"")
    ADD_DEFINITIONS("\"-DGUI_EXPORT=__declspec(dllimport)\"")
  ENDIF (MSVC)
ELSE (WIN32)
  # other compilers don't use that MSVC construct
  ADD_DEFINITIONS(-DCORE_EXPORT=)
  ADD_DEFINITIONS(-DGUI_EXPORT=)
ENDIF (WIN32)


#############################################################
# create qgsconfig.h

CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/qgsconfig.h.in ${CMAKE_SOURCE_DIR}/qgsconfig.h)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR})

#############################################################
# create qgssvnversion.h

# this is useful variable for developers
SET (QGIS_NO_SVNVERSION FALSE CACHE BOOL "Set to true if you don't want qgssvnversion.h to be regenerated every make")

IF (QGIS_NO_SVNVERSION)

  ADD_CUSTOM_TARGET(svnversion
                    echo '\#define QGSSVNVERSION \"\"' > qgssvnversion.h
                    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

ELSE (QGIS_NO_SVNVERSION)

  IF (WIN32)
    # TODO: create qgssvnversion.h properly
    ADD_CUSTOM_TARGET(svnversion echo "#define QGSSVNVERSION \"svn\"" > qgssvnversion.h
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
  ELSE (WIN32)
    ADD_CUSTOM_TARGET(svnversion ALL
  QGSSVNDEF='\#define QGSSVNVERSION \"'`svnversion .`'\"' &&
  if [ \"`grep QGSSVNVERSION qgssvnversion.h 2>/dev/null`\" != \"$$QGSSVNDEF\" ]; then 
  echo $$QGSSVNDEF > qgssvnversion.h  \; fi 
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  )

ENDIF (WIN32)

ENDIF (QGIS_NO_SVNVERSION)

#############################################################
# process subdirs

SUBDIRS(src doc images resources i18n tools)

IF (HAVE_PYTHON)
  SUBDIRS (python)
ENDIF (HAVE_PYTHON)


#############################################################
# install stuff

INSTALL (FILES AUTHORS SPONSORS
         DESTINATION ${QGIS_DATA_DIR}/doc)

# manual page... install also on windows?
INSTALL (FILES qgis.man
         DESTINATION ${CMAKE_INSTALL_PREFIX}/man/man1)
