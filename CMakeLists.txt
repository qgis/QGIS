
PROJECT(qgis0.9.1)

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

# Compile flag. Make it posible to turn it off.
SET (PEDANTIC FALSE CACHE BOOL "Determines if we should compile with -Wall -Werror.")

# whether unit tests should be build
SET (ENABLE_TESTS FALSE CACHE BOOL "Build unit tests?")

# hide this variable because building of python bindings might fail
# if set to other directory than expected
MARK_AS_ADVANCED(LIBRARY_OUTPUT_PATH)

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
IF (ENABLE_TESTS)
  SET( QT_USE_QTTEST  TRUE )
  ENABLE_TESTING()
ENDIF (ENABLE_TESTS)
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

IF (PEDANTIC)
  ADD_DEFINITIONS( -Wall -Werror )
ENDIF (PEDANTIC)

IF (CMAKE_BUILD_TYPE MATCHES Debug)
  ADD_DEFINITIONS(-DQGISDEBUG=1)
ENDIF (CMAKE_BUILD_TYPE MATCHES Debug)

#############################################################
# platform specific stuff

IF (WIN32)
  SET (DEFAULT_BIN_SUBDIR     .)
  SET (DEFAULT_DATA_SUBDIR    .)
  SET (DEFAULT_PLUGIN_SUBDIR  plugins)
  SET (DEFAULT_INCLUDE_SUBDIR	include)
  
  IF (MSVC)
    #tell msvc compiler to use main instead of winmain as the
    #application entry point
    SET(QT_USE_QTMAIN TRUE) 
    # Turn on defines for non standard maths stuff
    ADD_DEFINITIONS(-D_USE_MATH_DEFINES)

    # Turn off deprecation warnings
    ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS)
    ADD_DEFINITIONS(-D_CRT_NONSTDC_NO_WARNINGS)
    # Some file access stuff not defined in native win32
    # environment
    ADD_DEFINITIONS(-DF_OK=0)
    ADD_DEFINITIONS(-DX_OK=1)
    ADD_DEFINITIONS(-DW_OK=2)
    ADD_DEFINITIONS(-DR_OK=4)
    
    FILE(GLOB files "${CMAKE_SOURCE_DIR}/win_build/vcdeps/*.*")
    INSTALL(FILES ${files} DESTINATION .)
  ENDIF(MSVC)
ELSE (WIN32)

  IF (APPLE)
    # for Mac OS X, everything is put inside an application bundle
    SET (CMAKE_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX}/${PROJECT_NAME}.app/Contents/MacOS)
    # path for library references
    SET (CMAKE_INSTALL_NAME_DIR @executable_path/lib)
  ENDIF (APPLE)

  # common for MAC and UNIX
  SET (DEFAULT_BIN_SUBDIR     bin)
  SET (DEFAULT_DATA_SUBDIR    share/qgis)
  SET (DEFAULT_PLUGIN_SUBDIR  lib/qgis)
  SET (DEFAULT_INCLUDE_SUBDIR include/qgis)

  IF (UNIX AND NOT APPLE)
    ADD_DEFINITIONS(-DPREFIX=\\"${CMAKE_INSTALL_PREFIX}\\")
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
# user-changeable settings which can be used to customize
# layout of QGIS installation
# (default values are platform-specific)

SET (QGIS_BIN_SUBDIR     ${DEFAULT_BIN_SUBDIR}     CACHE STRING "Subdirectory where executables will be installed")
SET (QGIS_DATA_SUBDIR    ${DEFAULT_DATA_SUBDIR}    CACHE STRING "Subdirectory where QGIS data will be installed")
SET (QGIS_PLUGIN_SUBDIR  ${DEFAULT_PLUGIN_SUBDIR}  CACHE STRING "Subdirectory where plugins will be installed")
SET (QGIS_INCLUDE_SUBDIR ${DEFAULT_INCLUDE_SUBDIR} CACHE STRING "Subdirectory where header files will be installed")

# mark *_SUBDIR variables as advanced as this is not something
# that an average user would use
MARK_AS_ADVANCED (QGIS_BIN_SUBDIR QGIS_DATA_SUBDIR QGIS_PLUGIN_SUBDIR QGIS_INCLUDE_SUBDIR)

# full paths for the installation
SET (QGIS_BIN_DIR     ${CMAKE_INSTALL_PREFIX}/${QGIS_BIN_SUBDIR})
SET (QGIS_DATA_DIR    ${CMAKE_INSTALL_PREFIX}/${QGIS_DATA_SUBDIR})
SET (QGIS_PLUGIN_DIR  ${CMAKE_INSTALL_PREFIX}/${QGIS_PLUGIN_SUBDIR})
SET (QGIS_INCLUDE_DIR ${CMAKE_INSTALL_PREFIX}/${QGIS_INCLUDE_SUBDIR})

# manual page - makes sense only on unix systems
IF (UNIX)
  SET (DEFAULT_MANUAL_SUBDIR  man)
  SET (QGIS_MANUAL_SUBDIR  ${DEFAULT_MANUAL_SUBDIR}  CACHE STRING "Subdirectory where manual files will be installed")
  MARK_AS_ADVANCED (QGIS_MANUAL_SUBDIR)
  SET (QGIS_MANUAL_DIR  ${CMAKE_INSTALL_PREFIX}/${QGIS_MANUAL_SUBDIR})
ENDIF (UNIX)

#############################################################
# create qgsconfig.h

CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/cmake_templates/qgsconfig.h.in ${CMAKE_BINARY_DIR}/qgsconfig.h)
INSTALL(FILES ${CMAKE_BINARY_DIR}/qgsconfig.h DESTINATION ${QGIS_INCLUDE_DIR})

INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR})

#############################################################
# create qgssvnversion.h

FIND_FILE(SVN_MARKER entries PATHS ${CMAKE_SOURCE_DIR}/.svn)

IF (NOT SVN_MARKER)
  SET (SVN_MARKER ${CMAKE_SOURCE_DIR}/CMakeLists.txt)  # Dummy file
ENDIF (NOT SVN_MARKER)
  # Add a custom command to drive the svn script whenever the svn entries
  # file changes.
  CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/cmake_templates/svnscript.cmake.in_cmake
                ${CMAKE_CURRENT_BINARY_DIR}/svnscript.cmake
                @ONLY)

  ADD_CUSTOM_COMMAND (
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/qgssvnversion.h
    DEPENDS ${SVN_MARKER}
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/svnscript.cmake
)

ADD_CUSTOM_TARGET(svnversion ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/qgssvnversion.h)

#############################################################
# process subdirs

SUBDIRS(src doc images resources i18n tools)

IF (HAVE_PYTHON)
  SUBDIRS (python)
ENDIF (HAVE_PYTHON)

IF (ENABLE_TESTS)
  SUBDIRS(tests)
ENDIF (ENABLE_TESTS)

#############################################################
# install stuff

INSTALL (FILES AUTHORS SPONSORS
         DESTINATION ${QGIS_DATA_DIR}/doc)

# manual page - makes sense only on unix systems
IF (UNIX)
  INSTALL (FILES qgis.man
           DESTINATION ${QGIS_MANUAL_DIR}/man1)
ENDIF (UNIX)


#############################################################
# Uninstall stuff see: http://www.vtk.org/Wiki/CMake_FAQ
CONFIGURE_FILE(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake_templates/cmake_uninstall.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
  IMMEDIATE @ONLY)

ADD_CUSTOM_TARGET(uninstall
  "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")

#############################################################
# Enable packaging

INCLUDE(InstallRequiredSystemLibraries)

SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Quantum GIS")
SET(CPACK_PACKAGE_VENDOR "Tim Sutton")
SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README")
SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/COPYING")
SET(CPACK_PACKAGE_VERSION_MAJOR "0")
SET(CPACK_PACKAGE_VERSION_MINOR "9")
SET(CPACK_PACKAGE_VERSION_PATCH "1")
SET(CPACK_PACKAGE_INSTALL_DIRECTORY "Quantum GIS ${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}")
IF(WIN32 AND NOT UNIX)
  # There is a bug in NSI that does not handle full unix paths properly. Make
  # sure there is at least one set of four (4) backlasshes.
  SET(CPACK_PACKAGE_ICON "${CMAKE_SOURCE_DIR}/win_build\\\\sidebar.bmp")
  SET(CPACK_NSIS_INSTALLED_ICON_NAME "\\\\qgis.exe")
  SET(CPACK_NSIS_DISPLAY_NAME "${CPACK_PACKAGE_INSTALL_DIRECTORY} Quantum GIS")
  SET(CPACK_NSIS_HELP_LINK "http:\\\\\\\\qgis.org")
  SET(CPACK_NSIS_URL_INFO_ABOUT "http:\\\\\\\\qgis.org")
  SET(CPACK_NSIS_CONTACT "tim@linfiniti.com")
  SET(CPACK_NSIS_MODIFY_PATH ON)

#  SET(CPACK_NSIS_EXTRA_INSTALL_COMMANDS "  !include \\\"${CMAKE_SOURCE_DIR}\\\\win_build\\\\extra.nsh\\\"")
ELSE(WIN32 AND NOT UNIX)
  #SET(CPACK_STRIP_FILES "Quantum GIS")
  #SET(CPACK_SOURCE_STRIP_FILES "")
ENDIF(WIN32 AND NOT UNIX)
SET(CPACK_PACKAGE_EXECUTABLES "qgis" "QGIS")
INCLUDE(CPack)

