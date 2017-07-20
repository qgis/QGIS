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

MACRO (CHECK_GRASS_EXTRA_INCLUDE_DIRS)
  SET(GRASS_EXTRA_INCLUDE_DIRS ""
      CACHE STRING "Extra includes string used for GRASS")

  IF(UNIX AND EXISTS ${GRASS_INCLUDE_DIR}/Make/Platform.make
     AND "${GRASS_EXTRA_INCLUDE_DIRS}" STREQUAL "")

    FILE(READ ${GRASS_INCLUDE_DIR}/Make/Platform.make _platformfile)
    STRING(REGEX MATCH "INCLUDE_DIRS *= *[^\n]*" _config_includes "${_platformfile}")
    SET(_extra_includes "")
    IF(NOT "${_config_includes}" STREQUAL "")
      STRING(REGEX REPLACE "INCLUDE_DIRS *= *([^\n]*)" "\\1" _extra_includes "${_config_includes}")
    ENDIF()
    IF(NOT "${_extra_includes}" STREQUAL "")
      SET(GRASS_EXTRA_INCLUDE_DIRS ${_extra_includes}
          CACHE STRING "Extra includes string used for GRASS" FORCE)
    ENDIF()
  ENDIF()

  MARK_AS_ADVANCED (GRASS_EXTRA_INCLUDE_DIRS)
ENDMACRO (CHECK_GRASS_EXTRA_INCLUDE_DIRS)

# macro that checks for grass installation in specified directory

MACRO (CHECK_GRASS G_PREFIX)
  #MESSAGE(STATUS "Find GRASS  in ${G_PREFIX}")

  FIND_PATH(GRASS_INCLUDE_DIR grass/version.h ${G_PREFIX}/include DOC "Path to GRASS 7 include directory")

  #MESSAGE(STATUS "GRASS_INCLUDE_DIR = ${GRASS_INCLUDE_DIR}")

  IF(GRASS_INCLUDE_DIR AND EXISTS ${GRASS_INCLUDE_DIR}/grass/version.h)
    FILE(READ ${GRASS_INCLUDE_DIR}/grass/version.h VERSIONFILE)
    # We can avoid the following block using version_less version_equal and
    # version_greater. Are there compatibility problems?
    STRING(REGEX MATCH "[0-9]+\\.[0-9]+\\.[^ ]+" GRASS_VERSION ${VERSIONFILE})
    STRING(REGEX REPLACE "^([0-9]*)\\.[0-9]*\\..*$" "\\1" GRASS_MAJOR_VERSION ${GRASS_VERSION})
    STRING(REGEX REPLACE "^[0-9]*\\.([0-9]*)\\..*$" "\\1" GRASS_MINOR_VERSION ${GRASS_VERSION})
    STRING(REGEX REPLACE "^[0-9]*\\.[0-9]*\\.(.*)$" "\\1" GRASS_MICRO_VERSION ${GRASS_VERSION})
    # Add micro version too?
    # How to numerize RC versions?
    MATH( EXPR GRASS_NUM_VERSION "${GRASS_MAJOR_VERSION}*10000 + ${GRASS_MINOR_VERSION}*100")

    #MESSAGE(STATUS "GRASS_MAJOR_VERSION = ${GRASS_MAJOR_VERSION}")
    IF(GRASS_MAJOR_VERSION EQUAL 7)
        SET(GRASS_LIBRARIES_FOUND TRUE)
        SET(GRASS_LIB_NAMES gis dig2 dbmiclient dbmibase shape dgl rtree datetime linkm gproj)
        IF(GRASS_MAJOR_VERSION LESS 7 )
          LIST(APPEND GRASS_LIB_NAMES vect)
          LIST(APPEND GRASS_LIB_NAMES form)
          LIST(APPEND GRASS_LIB_NAMES I)
        ELSE(GRASS_MAJOR_VERSION LESS 7 )
          LIST(APPEND GRASS_LIB_NAMES vector)
          LIST(APPEND GRASS_LIB_NAMES raster)
          LIST(APPEND GRASS_LIB_NAMES imagery)
        ENDIF(GRASS_MAJOR_VERSION LESS 7 )

        FOREACH(LIB ${GRASS_LIB_NAMES})
          MARK_AS_ADVANCED ( GRASS_LIBRARY_${LIB} )

          SET(LIB_PATH NOTFOUND)
          # FIND_PATH and FIND_LIBRARY normally search standard locations
          # before the specified paths. To search non-standard paths first,
          # FIND_* is invoked first with specified paths and NO_DEFAULT_PATH
          # and then again with no specified paths to search the default
          # locations. When an earlier FIND_* succeeds, subsequent FIND_*s
          # searching for the same item do nothing. 
          FIND_LIBRARY(LIB_PATH NAMES grass_${LIB} PATHS ${G_PREFIX}/lib NO_DEFAULT_PATH)
          FIND_LIBRARY(LIB_PATH NAMES grass_${LIB} PATHS ${G_PREFIX}/lib)

          IF(LIB_PATH)
            SET(GRASS_LIBRARY_${LIB} ${LIB_PATH})
          ELSE(LIB_PATH)
            SET(GRASS_LIBRARY_${LIB} NOTFOUND)
            SET(GRASS_LIBRARIES_FOUND FALSE)
          ENDIF (LIB_PATH)
        ENDFOREACH(LIB)

        # LIB_PATH is only temporary variable, so hide it (is it possible to delete a variable?)
        UNSET(LIB_PATH CACHE)
    
        # Find off_t size
        IF( (GRASS_MAJOR_VERSION EQUAL 7) AND (GRASS_MINOR_VERSION GREATER 0) )
          SET(GRASS_TEST_MAPSET ${CMAKE_BINARY_DIR}/grass-location/PERMANENT)
          FILE(MAKE_DIRECTORY ${GRASS_TEST_MAPSET}) 
          FILE(WRITE ${GRASS_TEST_MAPSET}/DEFAULT_WIND "") 
          FILE(WRITE ${GRASS_TEST_MAPSET}/WIND "")
          # grass command is not in G_PREFIX but in some bin dir, so it must be in PATH
          SET(GRASS_EXE grass7${GRASS_MINOR_VERSION})
          #MESSAGE(STATUS "GRASS_EXE = ${GRASS_EXE}")
          EXECUTE_PROCESS(COMMAND ${GRASS_EXE} ${GRASS_TEST_MAPSET} --exec g.version -g
            COMMAND grep build_off_t_size 
            COMMAND sed "s/.*\\([0-9]\\).*/\\1/"
            ERROR_VARIABLE GRASS_TMP_ERROR
            OUTPUT_VARIABLE GRASS_OFF_T_SIZE
          )
          IF ( NOT ${GRASS_OFF_T_SIZE} STREQUAL "" )
            STRING(STRIP ${GRASS_OFF_T_SIZE} GRASS_OFF_T_SIZE)
          ENDIF()
          #MESSAGE(STATUS "GRASS_OFF_T_SIZE = ${GRASS_OFF_T_SIZE}")
        ENDIF( (GRASS_MAJOR_VERSION EQUAL 7) AND (GRASS_MINOR_VERSION GREATER 0) )

        IF ( "${GRASS_OFF_T_SIZE}" STREQUAL "" )
          IF(EXISTS ${GRASS_INCLUDE_DIR}/Make/Platform.make)
            FILE(READ ${GRASS_INCLUDE_DIR}/Make/Platform.make PLATFORMFILE)
            STRING(REGEX MATCH "LFS_CFLAGS *=[^\n]*" PLATFORM_LFS_CFLAGS ${PLATFORMFILE})
            IF ( NOT "${PLATFORM_LFS_CFLAGS}" STREQUAL "" )
              STRING(REGEX MATCH "_FILE_OFFSET_BITS=.." FILE_OFFSET_BITS ${PLATFORM_LFS_CFLAGS})
              #MESSAGE(STATUS "FILE_OFFSET_BITS = ${FILE_OFFSET_BITS}")
              IF ( NOT "${FILE_OFFSET_BITS}" STREQUAL "" )
                STRING(REGEX MATCH "[0-9][0-9]" FILE_OFFSET_BITS ${FILE_OFFSET_BITS})
                #MESSAGE(STATUS "FILE_OFFSET_BITS = ${FILE_OFFSET_BITS}")
                IF ( "${FILE_OFFSET_BITS}" STREQUAL "32" )
                  SET( GRASS_OFF_T_SIZE 4 )
                ELSEIF( "${FILE_OFFSET_BITS}" STREQUAL "64" )
                  SET( GRASS_OFF_T_SIZE 8 )
                ENDIF()        
              ENDIF()        
            ENDIF()        
          ENDIF()
        ENDIF()

        IF(GRASS_LIBRARIES_FOUND)
          SET(GRASS_FOUND TRUE)
          SET(GRASS_FOUND TRUE) # GRASS_FOUND is true if at least one version was found
          SET(GRASS_PREFIX ${G_PREFIX})
          CHECK_GRASS_EXTRA_INCLUDE_DIRS()
        ENDIF(GRASS_LIBRARIES_FOUND)
    ENDIF(GRASS_MAJOR_VERSION EQUAL 7)
  ENDIF(GRASS_INCLUDE_DIR AND EXISTS ${GRASS_INCLUDE_DIR}/grass/version.h)

  MARK_AS_ADVANCED ( GRASS_INCLUDE_DIR )
ENDMACRO (CHECK_GRASS)

###################################
# search for grass installations

#MESSAGE(STATUS "GRASS_FIND_VERSION = ")

# list of paths which to search - user's choice as first
SET (GRASS_PATHS ${GRASS_PREFIX} /usr/lib/grass /opt/grass $ENV{GRASS_PREFIX})

# os specific paths
IF (WIN32)
  LIST(APPEND GRASS_PATHS c:/msys/local)
ENDIF (WIN32)

IF (UNIX)
  IF (GRASS_FOUND)
    LIST(APPEND GRASS_PATHS /usr/lib64/grass70 /usr/lib/grass70 /usr/lib64/grass71 /usr/lib/grass71 /usr/lib64/grass72 /usr/lib/grass72)
  ENDIF (GRASS_FOUND)
ENDIF (UNIX)

IF (APPLE)
  IF (GRASS_FOUND)
    LIST(APPEND GRASS_PATHS
      /Applications/GRASS-7.0.app/Contents/MacOS
      /Applications/GRASS-7.1.app/Contents/MacOS
      /Applications/GRASS-7.2.app/Contents/MacOS
    )
  ENDIF (GRASS_FOUND)
  LIST(APPEND GRASS_PATHS /Applications/GRASS.app/Contents/Resources)
ENDIF (APPLE)

IF (WITH_GRASS)
  FOREACH (G_PREFIX ${GRASS_PATHS})
    IF (NOT GRASS_FOUND)
      CHECK_GRASS(${G_PREFIX})
    ENDIF (NOT GRASS_FOUND)
  ENDFOREACH (G_PREFIX)
ENDIF (WITH_GRASS)

###################################

IF (GRASS_FOUND)
   IF (NOT GRASS_FIND_QUIETLY)
      MESSAGE(STATUS "Found GRASS : ${GRASS_PREFIX} (${GRASS_VERSION}, off_t size = ${GRASS_OFF_T_SIZE})")
   ENDIF (NOT GRASS_FIND_QUIETLY)

ELSE (GRASS_FOUND)

   IF (WITH_GRASS)

     IF (GRASS_FIND_REQUIRED)
        MESSAGE(FATAL_ERROR "Could not find GRASS ")
     ELSE (GRASS_FIND_REQUIRED)
        MESSAGE(STATUS "Could not find GRASS ")
     ENDIF (GRASS_FIND_REQUIRED)

   ENDIF (WITH_GRASS)

ENDIF (GRASS_FOUND)
