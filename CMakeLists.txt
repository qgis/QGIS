
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


# try to configure and build GRASS plugin by default
SET (WITH_GRASS TRUE CACHE BOOL "Determines whether GRASS plugin should be built")
SET (GRASS_PREFIX "" CACHE PATH "Path to GRASS base directory")

# try to configure and build python bindings by default
SET (WITH_BINDINGS TRUE CACHE BOOL "Determines whether python bindings should be built")

# By default bindings will be installed only to QGIS directory
# Someone might want to install it to python site-packages directory
# as otherwise user has to use PYTHONPATH environemnt variable to add
# QGIS bindings to package search path
SET (BINDINGS_GLOBAL_INSTALL FALSE CACHE BOOL "Install bindings to global python directory? (might need root)")


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

# python support:
# - mapserver export tool
# - bindings
INCLUDE (Python) # file cmake/Python.cmake


IF (NOT PROJ_FOUND OR NOT SQLITE3_FOUND OR NOT GEOS_FOUND OR NOT GDAL_FOUND)
  MESSAGE (SEND_ERROR "Some dependencies were not found!")
ENDIF (NOT PROJ_FOUND OR NOT SQLITE3_FOUND OR NOT GEOS_FOUND OR NOT GDAL_FOUND)

IF (POSTGRES_FOUND)
  # following variable is used in qgsconfig.h
  SET (HAVE_POSTGRESQL TRUE)
ENDIF (POSTGRES_FOUND)


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
  SET (QGIS_INCLUDE_DIR ${CMAKE_INSTALL_PREFIX}/include)
  SET (QGIS_SOURCE_DIR ${CMAKE_SOURCE_DIR})

ELSE (WIN32)

  IF (APPLE)
    # for Mac OS X, everything is put inside an application bundle
    SET (CMAKE_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX}/${PROJECT_NAME}.app/Contents/MacOS)
    # path for library references
    SET (CMAKE_INSTALL_NAME_DIR @executable_path/lib)
  ENDIF (APPLE)

  # common for MAC and UNIX
  SET (QGIS_BIN_DIR    ${CMAKE_INSTALL_PREFIX}/bin)
  SET (QGIS_DATA_DIR   ${CMAKE_INSTALL_PREFIX}/share/qgis)
  SET (QGIS_PLUGIN_DIR ${CMAKE_INSTALL_PREFIX}/lib/qgis)
  SET(QGIS_INCLUDE_DIR "${CMAKE_INSTALL_PREFIX}/include/qgis")
  SET (QGIS_SOURCE_DIR ${CMAKE_SOURCE_DIR})


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

CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/qgsconfig.h.in ${CMAKE_BINARY_DIR}/qgsconfig.h)
INSTALL(FILES ${CMAKE_BINARY_DIR}/qgsconfig.h DESTINATION ${QGIS_INCLUDE_DIR})

INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR})

#############################################################
# create qgssvnversion.h

IF (WIN32)
  
  # TODO: create qgssvnversion.h properly
  ADD_CUSTOM_TARGET(svnversion ALL echo \\"#define QGSSVNVERSION \\\"svn_rev_unavailable\\\" \\" > qgssvnversion.h
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

ELSE (WIN32)

  ADD_CUSTOM_TARGET(svnversion ALL
  QGSSVNDEF='\#define QGSSVNVERSION \"'`svnversion ${QGIS_SOURCE_DIR}`'\"' &&
  if [ \"`grep QGSSVNVERSION qgssvnversion.h 2>/dev/null`\" != \"$$QGSSVNDEF\" ]; then 
  echo $$QGSSVNDEF > qgssvnversion.h  \; fi 
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

ENDIF (WIN32)

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


#############################################################
# Uninstall stuff see: http://www.vtk.org/Wiki/CMake_FAQ
CONFIGURE_FILE(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
  IMMEDIATE @ONLY)

ADD_CUSTOM_TARGET(uninstall
  "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")
