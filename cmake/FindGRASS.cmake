# Find GRASS
# ~~~~~~~~~~
# Copyright (c) 2007, Martin Dobias <wonder.sk at gmail.com>
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

# Macro that checks for extra include directories set during GRASS compilation.
# This helps for platforms where GRASS is built against dependencies in
# non-standard locations; like on Mac, where the system gettext is too old and
# GRASS is built off of gettext in /usr/local/opt, or some other custom prefix.
# Such includes may need found again when including some GRASS headers.

MACRO (CHECK_GRASS_EXTRA_INCLUDE_DIRS GRASS_VERSION)
  SET(GRASS_EXTRA_INCLUDE_DIRS${GRASS_VERSION} ""
      CACHE STRING "Extra includes string used for GRASS${GRASS_VERSION}")

  IF(UNIX AND EXISTS ${GRASS_INCLUDE_DIR${GRASS_VERSION}}/Make/Platform.make
     AND "${GRASS${GRASS_VERSION}_EXTRA_INCLUDE_DIRS}" STREQUAL "")

    FILE(READ ${GRASS_INCLUDE_DIR${GRASS_VERSION}}/Make/Platform.make _platformfile)
    STRING(REGEX MATCH "INCLUDE_DIRS *= *[^\n]*" _config_includes "${_platformfile}")
    SET(_extra_includes "")
    IF(NOT "${_config_includes}" STREQUAL "")
      STRING(REGEX REPLACE "INCLUDE_DIRS *= *([^\n]*)" "\\1" _extra_includes "${_config_includes}")
    ENDIF()
    IF(NOT "${_extra_includes}" STREQUAL "")
      SET(GRASS_EXTRA_INCLUDE_DIRS${GRASS_VERSION} ${_extra_includes}
          CACHE STRING "Extra includes string used for GRASS${GRASS_VERSION}" FORCE)
    ENDIF()
  ENDIF()

  MARK_AS_ADVANCED (GRASS_EXTRA_INCLUDE_DIRS${GRASS_VERSION})
ENDMACRO (CHECK_GRASS_EXTRA_INCLUDE_DIRS GRASS_VERSION)

# macro that checks for grass installation in specified directory

MACRO (CHECK_GRASS G_PREFIX)
  #MESSAGE(STATUS "Find GRASS ${GRASS_FIND_VERSION} in ${G_PREFIX}")

  FIND_PATH(GRASS_INCLUDE_DIR${GRASS_CACHE_VERSION} grass/version.h ${G_PREFIX}/include DOC "Path to GRASS ${GRASS_FIND_VERSION} include directory")

  #MESSAGE(STATUS "GRASS_INCLUDE_DIR${GRASS_CACHE_VERSION} = ${GRASS_INCLUDE_DIR${GRASS_CACHE_VERSION}}")

  IF(GRASS_INCLUDE_DIR${GRASS_CACHE_VERSION} AND EXISTS ${GRASS_INCLUDE_DIR${GRASS_CACHE_VERSION}}/grass/version.h)
    FILE(READ ${GRASS_INCLUDE_DIR${GRASS_CACHE_VERSION}}/grass/version.h VERSIONFILE)
    # We can avoid the following block using version_less version_equal and
    # version_greater. Are there compatibility problems?
    STRING(REGEX MATCH "[0-9]+\\.[0-9]+\\.[^ ]+" GRASS_VERSION${GRASS_FIND_VERSION} ${VERSIONFILE})
    STRING(REGEX REPLACE "^([0-9]*)\\.[0-9]*\\..*$" "\\1" GRASS_MAJOR_VERSION${GRASS_FIND_VERSION} ${GRASS_VERSION${GRASS_FIND_VERSION}})
    STRING(REGEX REPLACE "^[0-9]*\\.([0-9]*)\\..*$" "\\1" GRASS_MINOR_VERSION${GRASS_FIND_VERSION} ${GRASS_VERSION${GRASS_FIND_VERSION}})
    STRING(REGEX REPLACE "^[0-9]*\\.[0-9]*\\.(.*)$" "\\1" GRASS_MICRO_VERSION${GRASS_FIND_VERSION} ${GRASS_VERSION${GRASS_FIND_VERSION}})
    # Add micro version too?
    # How to numerize RC versions?
    MATH( EXPR GRASS_NUM_VERSION${GRASS_FIND_VERSION} "${GRASS_MAJOR_VERSION${GRASS_FIND_VERSION}}*10000 + ${GRASS_MINOR_VERSION${GRASS_FIND_VERSION}}*100")

    #MESSAGE(STATUS "GRASS_MAJOR_VERSION${GRASS_FIND_VERSION} = ${GRASS_MAJOR_VERSION${GRASS_FIND_VERSION}}")
    IF(GRASS_MAJOR_VERSION${GRASS_FIND_VERSION} EQUAL GRASS_FIND_VERSION)
        SET(GRASS_LIBRARIES_FOUND${GRASS_FIND_VERSION} TRUE)
        SET(GRASS_LIB_NAMES${GRASS_FIND_VERSION} gis dig2 dbmiclient dbmibase shape dgl rtree datetime linkm gproj)
        IF(GRASS_MAJOR_VERSION${GRASS_FIND_VERSION} LESS 7 )
          LIST(APPEND GRASS_LIB_NAMES${GRASS_FIND_VERSION} vect)
          LIST(APPEND GRASS_LIB_NAMES${GRASS_FIND_VERSION} form)
          LIST(APPEND GRASS_LIB_NAMES${GRASS_FIND_VERSION} I)
        ELSE(GRASS_MAJOR_VERSION${GRASS_FIND_VERSION} LESS 7 )
          LIST(APPEND GRASS_LIB_NAMES${GRASS_FIND_VERSION} vector)
          LIST(APPEND GRASS_LIB_NAMES${GRASS_FIND_VERSION} raster)
          LIST(APPEND GRASS_LIB_NAMES${GRASS_FIND_VERSION} imagery)
        ENDIF(GRASS_MAJOR_VERSION${GRASS_FIND_VERSION} LESS 7 )

        FOREACH(LIB ${GRASS_LIB_NAMES${GRASS_FIND_VERSION}})
          MARK_AS_ADVANCED ( GRASS_LIBRARY${GRASS_FIND_VERSION}_${LIB} )

          SET(LIB_PATH NOTFOUND)
          # FIND_PATH and FIND_LIBRARY normally search standard locations
          # before the specified paths. To search non-standard paths first,
          # FIND_* is invoked first with specified paths and NO_DEFAULT_PATH
          # and then again with no specified paths to search the default
          # locations. When an earlier FIND_* succeeds, subsequent FIND_*s
          # searching for the same item do nothing. 
          FIND_LIBRARY(LIB_PATH NAMES grass_${LIB} grass_${LIB}.${GRASS_MAJOR_VERSION${GRASS_FIND_VERSION}}.${GRASS_MINOR_VERSION${GRASS_FIND_VERSION}} PATHS ${G_PREFIX}/lib NO_DEFAULT_PATH)
          FIND_LIBRARY(LIB_PATH NAMES grass_${LIB} grass_${LIB}.${GRASS_MAJOR_VERSION${GRASS_FIND_VERSION}}.${GRASS_MINOR_VERSION${GRASS_FIND_VERSION}} PATHS ${G_PREFIX}/lib)

          IF(LIB_PATH)
            SET(GRASS_LIBRARY${GRASS_FIND_VERSION}_${LIB} ${LIB_PATH})
          ELSE(LIB_PATH)
            SET(GRASS_LIBRARY${GRASS_FIND_VERSION}_${LIB} NOTFOUND)
            SET(GRASS_LIBRARIES_FOUND${GRASS_FIND_VERSION} FALSE)
          ENDIF (LIB_PATH)
        ENDFOREACH(LIB)

        # LIB_PATH is only temporary variable, so hide it (is it possible to delete a variable?)
        UNSET(LIB_PATH CACHE)
    
        # Find off_t size
        IF( (GRASS_MAJOR_VERSION${GRASS_FIND_VERSION} EQUAL 7) AND (GRASS_MINOR_VERSION${GRASS_FIND_VERSION} GREATER 0) )
          SET(GRASS_TEST_MAPSET ${CMAKE_BINARY_DIR}/grass-location/PERMANENT)
          FILE(MAKE_DIRECTORY ${GRASS_TEST_MAPSET}) 
          FILE(WRITE ${GRASS_TEST_MAPSET}/DEFAULT_WIND "") 
          FILE(WRITE ${GRASS_TEST_MAPSET}/WIND "")
          # grass command is not in G_PREFIX but in some bin dir, so it must be in PATH
          SET(GRASS_EXE grass7${GRASS_MINOR_VERSION${GRASS_FIND_VERSION}})
          #MESSAGE(STATUS "GRASS_EXE = ${GRASS_EXE}")
          EXECUTE_PROCESS(COMMAND ${GRASS_EXE} ${GRASS_TEST_MAPSET} --exec g.version -g
            COMMAND grep build_off_t_size 
            COMMAND sed "s/.*\\([0-9]\\).*/\\1/"
            ERROR_VARIABLE GRASS_TMP_ERROR
            OUTPUT_VARIABLE GRASS_OFF_T_SIZE${GRASS_FIND_VERSION}
          )
          IF ( NOT ${GRASS_OFF_T_SIZE${GRASS_FIND_VERSION}} STREQUAL "" )
            STRING(STRIP ${GRASS_OFF_T_SIZE${GRASS_FIND_VERSION}} GRASS_OFF_T_SIZE${GRASS_FIND_VERSION})
          ENDIF()
          #MESSAGE(STATUS "GRASS_OFF_T_SIZE${GRASS_FIND_VERSION} = ${GRASS_OFF_T_SIZE${GRASS_FIND_VERSION}}")
        ENDIF( (GRASS_MAJOR_VERSION${GRASS_FIND_VERSION} EQUAL 7) AND (GRASS_MINOR_VERSION${GRASS_FIND_VERSION} GREATER 0) )

        IF ( "${GRASS_OFF_T_SIZE${GRASS_FIND_VERSION}}" STREQUAL "" )
          IF(EXISTS ${GRASS_INCLUDE_DIR${GRASS_CACHE_VERSION}}/Make/Platform.make)
            FILE(READ ${GRASS_INCLUDE_DIR${GRASS_CACHE_VERSION}}/Make/Platform.make PLATFORMFILE)
            STRING(REGEX MATCH "LFS_CFLAGS *=[^\n]*" PLATFORM_LFS_CFLAGS ${PLATFORMFILE})
            IF ( NOT "${PLATFORM_LFS_CFLAGS}" STREQUAL "" )
              STRING(REGEX MATCH "_FILE_OFFSET_BITS=.." FILE_OFFSET_BITS ${PLATFORM_LFS_CFLAGS})
              #MESSAGE(STATUS "FILE_OFFSET_BITS = ${FILE_OFFSET_BITS}")
              IF ( NOT "${FILE_OFFSET_BITS}" STREQUAL "" )
                STRING(REGEX MATCH "[0-9][0-9]" FILE_OFFSET_BITS ${FILE_OFFSET_BITS})
                #MESSAGE(STATUS "FILE_OFFSET_BITS = ${FILE_OFFSET_BITS}")
                IF ( "${FILE_OFFSET_BITS}" STREQUAL "32" )
                  SET( GRASS_OFF_T_SIZE${GRASS_FIND_VERSION} 4 )
                ELSEIF( "${FILE_OFFSET_BITS}" STREQUAL "64" )
                  SET( GRASS_OFF_T_SIZE${GRASS_FIND_VERSION} 8 )
                ENDIF()        
              ENDIF()        
            ENDIF()        
          ENDIF()
        ENDIF()

        IF(GRASS_LIBRARIES_FOUND${GRASS_FIND_VERSION})
          SET(GRASS_FOUND${GRASS_FIND_VERSION} TRUE)
          SET(GRASS_FOUND TRUE) # GRASS_FOUND is true if at least one version was found
          SET(GRASS_PREFIX${GRASS_CACHE_VERSION} ${G_PREFIX})
          CHECK_GRASS_EXTRA_INCLUDE_DIRS(${GRASS_FIND_VERSION})
        ENDIF(GRASS_LIBRARIES_FOUND${GRASS_FIND_VERSION})
    ENDIF(GRASS_MAJOR_VERSION${GRASS_FIND_VERSION} EQUAL GRASS_FIND_VERSION)
  ENDIF(GRASS_INCLUDE_DIR${GRASS_CACHE_VERSION} AND EXISTS ${GRASS_INCLUDE_DIR${GRASS_CACHE_VERSION}}/grass/version.h)

  MARK_AS_ADVANCED ( GRASS_INCLUDE_DIR${GRASS_CACHE_VERSION} )
ENDMACRO (CHECK_GRASS)

###################################
# search for grass installations

#MESSAGE(STATUS "GRASS_FIND_VERSION = ${GRASS_FIND_VERSION}")

# list of paths which to search - user's choice as first
SET (GRASS_PATHS ${GRASS_PREFIX${GRASS_CACHE_VERSION}} /usr/lib/grass /opt/grass $ENV{GRASS_PREFIX${GRASS_CACHE_VERSION}})

# os specific paths
IF (WIN32)
  LIST(APPEND GRASS_PATHS c:/msys/local)
ENDIF (WIN32)

IF (UNIX)
  IF (GRASS_FIND_VERSION EQUAL 7)
    IF (CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
        FOREACH (VERSION_MINOR 9 8 7 6 5 4 3 2 1 0)
            FOREACH (VERSION_BUILD 9 8 7 6 5 4 3 2 1 0)
                LIST (APPEND GRASS_PATHS /usr/local/grass-${GRASS_FIND_VERSION}.${VERSION_MINOR}.${VERSION_BUILD})
            ENDFOREACH (VERSION_BUILD)
        ENDFOREACH(VERSION_MINOR)
    ELSE (CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
        FOREACH (PATH /usr/lib64 /usr/lib)
            FOREACH (VERSION grass76, grass74, grass72, grass70)
                LIST(APPEND GRASS_PATHS "${PATH}/${VERSION}")
            ENDFOREACH (VERSION)
        ENDFOREACH (PATH)
    ENDIF (CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
   ENDIF (GRASS_FIND_VERSION EQUAL 7)
ENDIF (UNIX)

IF (APPLE)
  IF (GRASS_FIND_VERSION EQUAL 7)
    LIST(APPEND GRASS_PATHS
      /Applications/GRASS-7.6.app/Contents/MacOS
      /Applications/GRASS-7.4.app/Contents/MacOS
      /Applications/GRASS-7.2.app/Contents/MacOS
      /Applications/GRASS-7.0.app/Contents/MacOS
    )
  ENDIF ()
  LIST(APPEND GRASS_PATHS /Applications/GRASS.app/Contents/Resources)
ENDIF (APPLE)

IF (WITH_GRASS${GRASS_CACHE_VERSION})
  FOREACH (G_PREFIX ${GRASS_PATHS})
    IF (NOT GRASS_FOUND${GRASS_FIND_VERSION})
      CHECK_GRASS(${G_PREFIX})
    ENDIF (NOT GRASS_FOUND${GRASS_FIND_VERSION})
  ENDFOREACH (G_PREFIX)
ENDIF (WITH_GRASS${GRASS_CACHE_VERSION})

###################################

IF (GRASS_FOUND${GRASS_FIND_VERSION})
   IF (NOT GRASS_FIND_QUIETLY)
      MESSAGE(STATUS "Found GRASS ${GRASS_FIND_VERSION}: ${GRASS_PREFIX${GRASS_CACHE_VERSION}} (${GRASS_VERSION${GRASS_FIND_VERSION}}, off_t size = ${GRASS_OFF_T_SIZE${GRASS_FIND_VERSION}})")
   ENDIF (NOT GRASS_FIND_QUIETLY)

ELSE (GRASS_FOUND${GRASS_FIND_VERSION})

   IF (WITH_GRASS${GRASS_CACHE_VERSION})

     IF (GRASS_FIND_REQUIRED)
        MESSAGE(FATAL_ERROR "Could not find GRASS ${GRASS_FIND_VERSION}")
     ELSE (GRASS_FIND_REQUIRED)
        MESSAGE(STATUS "Could not find GRASS ${GRASS_FIND_VERSION}")
     ENDIF (GRASS_FIND_REQUIRED)

   ENDIF (WITH_GRASS${GRASS_CACHE_VERSION})

ENDIF (GRASS_FOUND${GRASS_FIND_VERSION})
