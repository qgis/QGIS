# Find SFCGAL
# ~~~~~~~~~
# Copyright (c) 2024, De Mezzo Benoit <benoit dot de dot mezzo at oslandia dot com>
# Copyright (c) 2024, Felder Jean <jean dot felder at oslandia dot com>
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#
# SFCGAL_DIR variable can be set to define a custom sfcgal installation path
#
# Once run this will define:
#
# SFCGAL_FOUND       = system has SFCGAL lib
#
# SFCGAL_LIBRARY     = full path to the library
#
# SFCGAL_INCLUDE_DIR      = where to find headers

find_package(SFCGAL CONFIG)
if(NOT SFCGAL_FOUND)
  # Fallback logic for SFCGAL < 2.1, as soon as we switch to SFCGAL>=2.1 this file (FindSFCGAL.cmake) can be deleted

  IF (NOT SFCGAL_INCLUDE_DIR OR NOT SFCGAL_LIBRARY)
    IF(WIN32)
      IF (MINGW)
        FIND_PATH(SFCGAL_INCLUDE_DIR
          SFCGAL/capi/sfcgal_c.h
          /usr/local/include
          /usr/include
          c:/msys/local/include
          ${SFCGAL_DIR}/include
          PATH_SUFFIXES sfcgal
        )

        FIND_LIBRARY(SFCGAL_LIBRARY NAMES SFCGAL
          PATHS
          /usr/local/lib
          /usr/lib
          c:/msys/local/lib
          ${SFCGAL_DIR}/lib
        )
      ENDIF (MINGW)

      IF (MSVC)
        FIND_PATH(SFCGAL_INCLUDE_DIR
          SFCGAL/capi/sfcgal_c.h
          "$ENV{LIB_DIR}/include"
          $ENV{INCLUDE}
          ${SFCGAL_DIR}/include
        )

        FIND_LIBRARY(SFCGAL_LIBRARY NAMES SFCGAL SFCGAL_i
          PATHS
  	  "$ENV{LIB_DIR}/lib"
          $ENV{LIB}
          /usr/lib
          c:/msys/local/lib
          ${SFCGAL_DIR}/lib
        )
      ENDIF (MSVC)

    ELSEIF(APPLE AND QGIS_MAC_DEPS_DIR)
      FIND_PATH(SFCGAL_INCLUDE_DIR
        SFCGAL/capi/sfcgal_c.h
        "$ENV{LIB_DIR}/include"
        ${SFCGAL_DIR}/include
      )

      FIND_LIBRARY(SFCGAL_LIBRARY NAMES SFCGAL
        PATHS
        "$ENV{LIB_DIR}/lib"
        ${SFCGAL_DIR}/include
      )

    ELSE(WIN32)

      # try to use framework on mac
      # want clean framework path, not unix compatibility path
      IF (APPLE)
        IF (CMAKE_FIND_FRAMEWORK MATCHES "FIRST"
          OR CMAKE_FRAMEWORK_PATH MATCHES "ONLY"
          OR NOT CMAKE_FIND_FRAMEWORK)
          SET (CMAKE_FIND_FRAMEWORK_save ${CMAKE_FIND_FRAMEWORK} CACHE STRING "" FORCE)
          SET (CMAKE_FIND_FRAMEWORK "ONLY" CACHE STRING "" FORCE)
          FIND_LIBRARY(SFCGAL_LIBRARY SFCGAL)
          IF (SFCGAL_LIBRARY)
            # they're all the same in a framework
            SET (SFCGAL_INCLUDE_DIR ${SFCGAL_LIBRARY}/Headers CACHE PATH "Path to a file.")
            # set SFCGAL_CONFIG to make later test happy, not used here, may not exist
            SET (SFCGAL_CONFIG ${SFCGAL_LIBRARY}/unix/bin/sfcgal-config CACHE FILEPATH "Path to a program.")
            # version in info.plist
            GET_VERSION_PLIST (${SFCGAL_LIBRARY}/Resources/Info.plist SFCGAL_VERSION)
          ENDIF (SFCGAL_LIBRARY)
          SET (CMAKE_FIND_FRAMEWORK ${CMAKE_FIND_FRAMEWORK_save} CACHE STRING "" FORCE)
        ENDIF ()
      ENDIF (APPLE)

      IF(CYGWIN)
        FIND_LIBRARY(SFCGAL_LIBRARY NAMES SFCGAL
          PATHS
          /usr/lib
          /usr/local/lib
          ${SFCGAL_DIR}/lib
        )
      ENDIF(CYGWIN)

      # SFCGAL_INCLUDE_DIR or SFCGAL_LIBRARY has not been found, try to set SFCGAL_CONFIG
      IF (NOT SFCGAL_INCLUDE_DIR OR NOT SFCGAL_LIBRARY)
        SET(SFCGAL_CONFIG_PREFER_PATH "$ENV{SFCGAL_HOME}/bin" CACHE STRING "preferred path to SFCGAL (sfcgal-config)")
        FIND_PROGRAM(SFCGAL_CONFIG sfcgal-config
          ${SFCGAL_CONFIG_PREFER_PATH}
          ${SFCGAL_DIR}/bin
          $ENV{LIB_DIR}/bin
          /usr/local/bin/
          /usr/bin/
        )
      ENDIF (NOT SFCGAL_INCLUDE_DIR OR NOT SFCGAL_LIBRARY)

    ENDIF(WIN32)
  ENDIF (NOT SFCGAL_INCLUDE_DIR OR NOT SFCGAL_LIBRARY)

  IF (SFCGAL_CONFIG)
    # set INCLUDE_DIR to prefix+include
    execute_process(COMMAND ${SFCGAL_CONFIG} --prefix
      OUTPUT_STRIP_TRAILING_WHITESPACE OUTPUT_VARIABLE SFCGAL_PREFIX)

    FIND_PATH(SFCGAL_INCLUDE_DIR
      SFCGAL/capi/sfcgal_c.h
      ${SFCGAL_PREFIX}/include
      /usr/local/include
      /usr/include
    )

    ## extract link dirs for rpath
    execute_process(COMMAND ${SFCGAL_CONFIG} --libs
      OUTPUT_STRIP_TRAILING_WHITESPACE OUTPUT_VARIABLE SFCGAL_CONFIG_LIBS )

    ## split off the link dirs (for rpath)
    ## use regular expression to match wildcard equivalent "-L*<endchar>"
    ## with <endchar> is a space or a semicolon
    STRING(REGEX MATCHALL "[-][L]([^ ;])+"
      SFCGAL_LINK_DIRECTORIES_WITH_PREFIX
      "${SFCGAL_CONFIG_LIBS}" )

    IF (SFCGAL_LINK_DIRECTORIES_WITH_PREFIX)
      STRING(REGEX REPLACE "[-][L]" "" SFCGAL_LINK_DIRECTORIES ${SFCGAL_LINK_DIRECTORIES_WITH_PREFIX} )
    ENDIF (SFCGAL_LINK_DIRECTORIES_WITH_PREFIX)

    FIND_LIBRARY(SFCGAL_LIBRARY NAMES SFCGAL libSFCGAL SFCGALd libSFCGALd
      PATHS
      ${SFCGAL_LINK_DIRECTORIES}/lib
      ${SFCGAL_LINK_DIRECTORIES}
    )

  ENDIF (SFCGAL_CONFIG)


  IF (SFCGAL_INCLUDE_DIR AND SFCGAL_LIBRARY)
    SET(SFCGAL_FOUND TRUE)
  ENDIF (SFCGAL_INCLUDE_DIR AND SFCGAL_LIBRARY)

  IF (SFCGAL_FOUND)
    IF (NOT SFCGAL_FIND_QUIETLY)
      FILE(READ ${SFCGAL_INCLUDE_DIR}/SFCGAL/version.h sfcgal_version)
      STRING(REGEX REPLACE "^.*SFCGAL_VERSION +\"([^\"]+)\".*$" "\\1" SFCGAL_VERSION "${sfcgal_version}")

      MESSAGE(STATUS "Found SFCGAL: ${SFCGAL_LIBRARY} (${SFCGAL_VERSION})")
    ENDIF (NOT SFCGAL_FIND_QUIETLY)
    add_library(SFCGAL::SFCGAL UNKNOWN IMPORTED)
    target_link_libraries(SFCGAL::SFCGAL INTERFACE ${SFCGAL_LIBRARY})
    target_include_directories(SFCGAL::SFCGAL INTERFACE ${SFCGAL_INCLUDE_DIR})
    set_target_properties(SFCGAL::SFCGAL PROPERTIES IMPORTED_LOCATION ${SFCGAL_LIBRARY})
  ELSE (SFCGAL_FOUND)

    MESSAGE(SFCGAL_INCLUDE_DIR=${SFCGAL_INCLUDE_DIR})
    MESSAGE(SFCGAL_LIBRARY=${SFCGAL_LIBRARY})
    MESSAGE(FATAL_ERROR "Could not find SFCGAL")

  ENDIF (SFCGAL_FOUND)
endif()
