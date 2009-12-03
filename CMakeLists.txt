SET(CPACK_PACKAGE_VERSION_MAJOR "1")
SET(CPACK_PACKAGE_VERSION_MINOR "4")
SET(CPACK_PACKAGE_VERSION_PATCH "0")
SET(COMPLETE_VERSION ${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH})
SET(RELEASE_NAME "Trunk")
SET(PROJECT_VERSION ${COMPLETE_VERSION})
PROJECT(qgis${PROJECT_VERSION})
# Note the version no is Mmmpp for Major/minor/patch, 0-padded, thus '10100' for 1.1.0
SET(QGIS_VERSION_INT 10400)

# TODO:
# - install includes for libs
# - nice output when configured
# - rename *.ui files to have the same filename as their implementation
#   e.g. instead of blahblahbase.ui use blahblah.ui
#   because it's more common in Qt4 
# Note on last point above by Tim Sutton - I prefer to have the base suffix
# as it indicates the ui generated class will be a base class for the widget.

#############################################################
# CMake settings

CMAKE_MINIMUM_REQUIRED(VERSION 2.4.3)

SET(CMAKE_COLOR_MAKEFILE ON)

# set path to additional CMake modules
SET(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

# in generated makefiles use relative paths so the project dir is moveable
# Note commented out since it cause problems but it would be nice to resolve these and enable
# 
# issue is caused by INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR}) near the end of this file generating incorrect path 
#SET (CMAKE_USE_RELATIVE_PATHS ON)

# it's possible to set PLUGINS_ALSO_BINARIES to TRUE
# then some plugins that can run as standalone apps will be built
# also as standalone apps
SET (PLUGINS_ALSO_BINARIES FALSE)


# try to configure and build GRASS plugin by default
SET (WITH_GRASS TRUE CACHE BOOL "Determines whether GRASS plugin should be built")
IF (WITH_GRASS)
  SET (GRASS_PREFIX "" CACHE PATH "Path to GRASS base directory")
ENDIF (WITH_GRASS)

# try to configure and build POSTGRESQL support
SET (WITH_POSTGRESQL TRUE CACHE BOOL "Determines whether POSTGRESQL support should be built")
IF (WITH_POSTGRESQL)
  SET (POSTGRESQL_PREFIX "" CACHE PATH "Path to POSTGRESQL base directory")
ENDIF (WITH_POSTGRESQL)

SET (WITH_SPATIALITE TRUE CACHE BOOL "Determines whether SPATIALITE support should be built")

IF (WITH_SPATIALITE)
  SET (WITH_INTERNAL_SPATIALITE TRUE CACHE BOOL "Determines whether SPATIALITE support should be built internally")
  IF(WITH_INTERNAL_SPATIALITE)
    SET(SPATIALITE_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/src/core/spatialite/headers)
    SET(HAVE_SPATIALITE TRUE)
    IF(WIN32 OR APPLE)
      FIND_PACKAGE(Iconv REQUIRED)
    ENDIF(WIN32 OR APPLE)
  ENDIF (WITH_INTERNAL_SPATIALITE)
ENDIF (WITH_SPATIALITE)

# try to configure and build python bindings by default
SET (WITH_BINDINGS TRUE CACHE BOOL "Determines whether python bindings should be built")
IF (WITH_BINDINGS)
  # By default bindings will be installed only to QGIS directory
  # Someone might want to install it to python site-packages directory
  # as otherwise user has to use PYTHONPATH environemnt variable to add
  # QGIS bindings to package search path
  SET (BINDINGS_GLOBAL_INSTALL FALSE CACHE BOOL "Install bindings to global python directory? (might need root)")
ENDIF (WITH_BINDINGS)

# Compile flag. Make it posible to turn it off.
SET (PEDANTIC FALSE CACHE BOOL "Determines if we should compile in pedantic mode.")

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

# we don't use cblas
SET(DONT_LINK_CBLAS TRUE)

IF(NOT WIN32)
  INCLUDE(CheckFunctionExists)
  CHECK_FUNCTION_EXISTS(openpty OPENPTY_IN_LIBC)
  IF(NOT OPENPTY_IN_LIBC)
    SET(CMAKE_REQUIRED_INCLUDES util.h)
    SET(CMAKE_REQUIRED_LIBRARIES util)
    CHECK_FUNCTION_EXISTS(openpty NEED_LIBUTIL)
    IF(NEED_LIBUTIL)
      SET(OPENPTY_LIBRARY util)
    ELSE(NEED_LIBUTIL)
      MESSAGE (SEND_ERROR "openpty not found!")
    ENDIF(NEED_LIBUTIL)
  ENDIF(NOT OPENPTY_IN_LIBC)
ENDIF(NOT WIN32)

# required
FIND_PACKAGE(Proj)
FIND_PACKAGE(Expat)    # GPS importer plugin
FIND_PACKAGE(GSL)      # Georeferencer
FIND_PACKAGE(GEOS)
FIND_PACKAGE(GDAL)

IF (NOT WITH_INTERNAL_SPATIALITE)
  FIND_PACKAGE(Sqlite3)
  IF (NOT SQLITE3_FOUND)
    MESSAGE (SEND_ERROR "sqlite3 dependency was not found (try compiling with internal spatialite)!")
  ENDIF (NOT SQLITE3_FOUND)
ENDIF (NOT WITH_INTERNAL_SPATIALITE)

# optional
IF (WITH_POSTGRESQL)
  FIND_PACKAGE(Postgres) # PostgreSQL provider, SPIT plugin
ENDIF (WITH_POSTGRESQL)
IF (WITH_GRASS)
  FIND_PACKAGE(GRASS)    # GRASS plugin
ENDIF (WITH_GRASS)

IF (WITH_SPATIALITE AND NOT WITH_INTERNAL_SPATIALITE)
  FIND_PACKAGE(SPATIALITE)
ENDIF (WITH_SPATIALITE AND NOT WITH_INTERNAL_SPATIALITE)

IF (WITH_BINDINGS)
  # python support:
  # - mapserver export tool
  # - bindings
  INCLUDE (Python) # file cmake/Python.cmake
ENDIF (WITH_BINDINGS)

IF (NOT PROJ_FOUND OR NOT GEOS_FOUND OR NOT GDAL_FOUND)
  MESSAGE (SEND_ERROR "Some dependencies were not found!")
ENDIF (NOT PROJ_FOUND OR NOT GEOS_FOUND OR NOT GDAL_FOUND)

IF (POSTGRES_FOUND)
  # following variable is used in qgsconfig.h
  SET (HAVE_POSTGRESQL TRUE)
ENDIF (POSTGRES_FOUND)

IF (SPATIALITE_FOUND)
  # following variable is used in qgsconfig.h
  SET (HAVE_SPATIALITE TRUE)
ENDIF (SPATIALITE_FOUND)

#############################################################
# search for Qt4
FIND_PACKAGE(Qt4 REQUIRED)

SET( QT_USE_QTXML      TRUE )
SET( QT_USE_QTNETWORK  TRUE )
SET( QT_USE_QTSVG      TRUE )
SET( QT_USE_QTSQL      TRUE )
IF (ENABLE_TESTS)
  SET( QT_USE_QTTEST  TRUE )
  ENABLE_TESTING()
ENDIF (ENABLE_TESTS)


INCLUDE( ${QT_USE_FILE} )

# Disable automatic conversion from QString to ASCII 8-bit strings (char *)
# (Keeps code compatible with Qt/Mac/64bit)
ADD_DEFINITIONS(-DQT_NO_CAST_TO_ASCII)

FIND_PROGRAM(QT_LRELEASE_EXECUTABLE
    NAMES lrelease
    PATHS ${QT_BINARY_DIR}
    NO_DEFAULT_PATH
    )

#############################################################
# enable warnings

IF (PEDANTIC)
  MESSAGE (STATUS "Pedantic compiler settings enabled")
  IF(MSVC)
    ADD_DEFINITIONS( /W4 )

    # disable warnings
    ADD_DEFINITIONS( /wd4100 )  # unused formal parameters
    ADD_DEFINITIONS( /wd4127 )  # constant conditional expressions (used in Qt template classes)
    ADD_DEFINITIONS( /wd4510 )  # default constructor could not be generated (sqlite3_index_info, QMap)
    ADD_DEFINITIONS( /wd4512 )  # assignment operator could not be generated (sqlite3_index_info)
    ADD_DEFINITIONS( /wd4610 )  # user defined constructor required (sqlite3_index_info)
  ELSE (MSVC)
    ADD_DEFINITIONS( -Wall -Wno-long-long -Wformat-security )
    # Qt produces lots of warnings with strict aliasing (as of Qt 4.4.0 & GCC 4.3)
    # ADD_DEFINITIONS( -fstrict-aliasing -Wstrict-aliasing=1 )
  ENDIF (MSVC)
ENDIF (PEDANTIC)

IF (CMAKE_BUILD_TYPE MATCHES Debug OR CMAKE_BUILD_TYPE MATCHES RelWithDebInfo)
  MESSAGE (STATUS "Debug output enabled")
  ADD_DEFINITIONS(-DQGISDEBUG=1)
ENDIF (CMAKE_BUILD_TYPE MATCHES Debug OR CMAKE_BUILD_TYPE MATCHES RelWithDebInfo)

#############################################################
# platform specific stuff

IF (WIN32)
  SET (DEFAULT_LIB_SUBDIR     lib)
  SET (DEFAULT_DATA_SUBDIR    .)
  SET (DEFAULT_PLUGIN_SUBDIR  plugins)
  SET (DEFAULT_INCLUDE_SUBDIR include)
  
  IF (MSVC)
    SET (DEFAULT_BIN_SUBDIR bin)
    # put all the build products into a single directory
    # under build (doesnt affect install target) to make for
    # easier debugging.
    #tell msvc compiler to use main instead of winmain as the
    #application entry point
    #SET(QT_USE_QTMAIN TRUE) 
    # Turn on defines for non standard maths stuff
    ADD_DEFINITIONS(-D_USE_MATH_DEFINES)

    # Turn off deprecation warnings
    ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS)
    ADD_DEFINITIONS(-D_CRT_NONSTDC_NO_WARNINGS)
    
    IF (CMAKE_BUILD_TYPE MATCHES Debug OR CMAKE_BUILD_TYPE MATCHES RelWithDebInfo)
      MESSAGE (STATUS "Generating browse files")
      ADD_DEFINITIONS( /FR )
    ENDIF (CMAKE_BUILD_TYPE MATCHES Debug OR CMAKE_BUILD_TYPE MATCHES RelWithDebInfo)

    IF (INSTALL_DEPS)
      INSTALL(DIRECTORY ${INSTALL_DEPS} DESTINATION .)
    ENDIF (INSTALL_DEPS)
  ELSE(MSVC)
    SET (DEFAULT_BIN_SUBDIR .)
  ENDIF(MSVC)
ELSE (WIN32)

  IF (APPLE)
    # for Mac OS X, everything is put inside an application bundle
    SET (CMAKE_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX}/${PROJECT_NAME}.app/Contents/MacOS)
    # path for library references
    SET (CMAKE_INSTALL_NAME_DIR @executable_path/lib)
    #this will define ${APP_SERVICES_LIBRARY}
    FIND_LIBRARY(APP_SERVICES_LIBRARY ApplicationServices )
  ENDIF (APPLE)

  # common for MAC and UNIX
  SET (DEFAULT_BIN_SUBDIR     bin)
  SET (DEFAULT_LIB_SUBDIR     lib${LIB_SUFFIX})
  SET (DEFAULT_DATA_SUBDIR    share/qgis)
  SET (DEFAULT_PLUGIN_SUBDIR  lib${LIB_SUFFIX}/qgis)
  SET (DEFAULT_INCLUDE_SUBDIR include/qgis)

ENDIF (WIN32)


#assume we have excaped compiler directives
#eventually we want to change this to new
#since we dont need to jump through so many 
#hoops to escape compiler directives then
IF(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
  cmake_policy(SET CMP0005 OLD)
ENDIF(COMMAND cmake_policy)

IF (WIN32)
  # expect that classes are being imported
  # Note: MSVC doesn't like when the macros are quotes
  # and MSYS doesn't like them unqouted (bacause of braces)
  IF (MSVC)
    ADD_DEFINITIONS("-DCORE_EXPORT=__declspec(dllimport)")
    ADD_DEFINITIONS("-DGUI_EXPORT=__declspec(dllimport)")
    ADD_DEFINITIONS("-DPYTHON_EXPORT=__declspec(dllimport)")
    ADD_DEFINITIONS("-DANALYSIS_EXPORT=__declspec(dllimport)")
  ELSE (MSVC)
    ADD_DEFINITIONS("\"-DCORE_EXPORT=__declspec(dllimport)\"")
    ADD_DEFINITIONS("\"-DGUI_EXPORT=__declspec(dllimport)\"")
    ADD_DEFINITIONS("\"-DPYTHON_EXPORT=__declspec(dllimport)\"")
    ADD_DEFINITIONS("\"-DANALYSIS_EXPORT=__declspec(dllimport)\"")
  ENDIF (MSVC)
ELSE (WIN32)
  # other compilers don't use that MSVC construct
  ADD_DEFINITIONS(-DCORE_EXPORT=)
  ADD_DEFINITIONS(-DGUI_EXPORT=)
  ADD_DEFINITIONS(-DPYTHON_EXPORT=)
  ADD_DEFINITIONS(-DANALYSIS_EXPORT=)
ENDIF (WIN32)

#############################################################
# user-changeable settings which can be used to customize
# layout of QGIS installation
# (default values are platform-specific)

SET (QGIS_BIN_SUBDIR     ${DEFAULT_BIN_SUBDIR}     CACHE STRING "Subdirectory where executables will be installed")
SET (QGIS_LIB_SUBDIR     ${DEFAULT_LIB_SUBDIR}     CACHE STRING "Subdirectory where libraries will be installed")
SET (QGIS_DATA_SUBDIR    ${DEFAULT_DATA_SUBDIR}    CACHE STRING "Subdirectory where QGIS data will be installed")
SET (QGIS_PLUGIN_SUBDIR  ${DEFAULT_PLUGIN_SUBDIR}  CACHE STRING "Subdirectory where plugins will be installed")
SET (QGIS_INCLUDE_SUBDIR ${DEFAULT_INCLUDE_SUBDIR} CACHE STRING "Subdirectory where header files will be installed")

# mark *_SUBDIR variables as advanced as this is not something
# that an average user would use
MARK_AS_ADVANCED (QGIS_BIN_SUBDIR QGIS_LIB_SUBDIR QGIS_DATA_SUBDIR QGIS_PLUGIN_SUBDIR QGIS_INCLUDE_SUBDIR)

# full paths for the installation
SET (QGIS_BIN_DIR     ${QGIS_BIN_SUBDIR})
SET (QGIS_LIB_DIR     ${QGIS_LIB_SUBDIR})
SET (QGIS_DATA_DIR    ${QGIS_DATA_SUBDIR})
SET (QGIS_PLUGIN_DIR  ${QGIS_PLUGIN_SUBDIR})
SET (QGIS_INCLUDE_DIR ${QGIS_INCLUDE_SUBDIR})

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

# Added by Jef to prevent python core and gui libs linking to other qgisCore and qgisGui libs
# that may be in the same install prefix
LINK_DIRECTORIES(${CMAKE_BINARY_DIR}/src/core ${CMAKE_BINARY_DIR}/src/gui)

#############################################################
# create qgssvnversion.h

FIND_FILE(SVN_MARKER entries PATHS ${CMAKE_SOURCE_DIR}/.svn)

IF (SVN_MARKER)
  # See if we have svn installed
  FIND_PROGRAM(SVNVERSION svnversion PATHS c:/cygwin/bin)

  IF(SVNVERSION)
    IF(MSVC)
	ADD_CUSTOM_COMMAND(
		OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/qgssvnversion.h
		COMMAND for /f usebackq %%a in "(`\"${SVNVERSION}\"`)" do echo \#define QGSSVNVERSION \"%%a\" >${CMAKE_CURRENT_BINARY_DIR}/qgssvnversion.h
		MAIN_DEPENDENCY ${SVN_MARKER}
		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
	)
    ELSE(MSVC)
	ADD_CUSTOM_COMMAND(
		OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/qgssvnversion.h
		COMMAND echo \\\#define QGSSVNVERSION \\\"`${SVNVERSION}`\\\" >${CMAKE_CURRENT_BINARY_DIR}/qgssvnversion.h
		MAIN_DEPENDENCY ${SVN_MARKER}
		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
	)
    ENDIF(MSVC)
  ELSE(SVNVERSION)
    MESSAGE(STATUS "svnversion not found - version will be unknown")
    FILE(WRITE ${CMAKE_CURRENT_BINARY_DIR}/qgssvnversion.h "#define QGSSVNVERSION \"unknown\"")
  ENDIF (SVNVERSION)
ELSE (SVN_MARKER)
  FILE(WRITE ${CMAKE_CURRENT_BINARY_DIR}/qgssvnversion.h "#define QGSSVNVERSION \"exported\"")
ENDIF (SVN_MARKER)

ADD_CUSTOM_TARGET(svnversion ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/qgssvnversion.h)

#############################################################
# process subdirs

SUBDIRS(src doc images resources i18n)

IF (HAVE_PYTHON AND WITH_BINDINGS)
  SUBDIRS (python)
ELSE (HAVE_PYTHON AND WITH_BINDINGS)
  MESSAGE (STATUS "Python not being built")
ENDIF (HAVE_PYTHON AND WITH_BINDINGS)

IF (ENABLE_TESTS)
  #create a variable to specify where our test data is
  #so that unit tests can use TEST_DATA_DIR to locate
  #the test data. See CMakeLists in test dirs for more info
  SET (TEST_DATA_DIR "${CMAKE_CURRENT_SOURCE_DIR}/tests/testdata")
  SUBDIRS(tests)
ENDIF (ENABLE_TESTS)

#############################################################
# install stuff

INSTALL (FILES AUTHORS CONTRIBUTORS SPONSORS DONORS TRANSLATORS INSTALL CODING
         DESTINATION ${QGIS_DATA_DIR}/doc)

# manual page - makes sense only on unix systems
IF (UNIX)
  INSTALL (FILES qgis.1 qgis_help.1
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
SET(CPACK_PACKAGE_VENDOR "Open Source Geospatial Foundation")
SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README")
SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/COPYING")
SET(CPACK_PACKAGE_INSTALL_DIRECTORY "Quantum GIS ${COMPLETE_VERSION}")
IF(WIN32 AND NOT UNIX)
  # There is a bug in NSI that does not handle full unix paths properly. Make
  # sure there is at least one set of four (4) backslashes.
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
