# Find GRASS
# ~~~~~~~~~~
# Copyright (c) 2007, Martin Dobias <wonder.sk at gmail.com>
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

# macro that checks for grass installation in specified directory

MACRO (CHECK_GRASS G_PREFIX)

  FIND_PATH (GRASS_INCLUDE_DIR grass/version.h ${G_PREFIX}/include)

  SET (GRASS_LIBRARIES_FOUND TRUE)
  SET (GRASS_LIB_NAMES gis vect dig2 dbmiclient dbmibase shape dgl rtree datetime linkm form gproj)

  FOREACH (LIB ${GRASS_LIB_NAMES})
    MARK_AS_ADVANCED ( GRASS_LIBRARY_${LIB} )

    SET(LIB_PATH NOTFOUND)
    FIND_LIBRARY(LIB_PATH NAMES grass_${LIB} PATHS ${G_PREFIX}/lib NO_DEFAULT_PATH)

    IF (LIB_PATH)
      SET (GRASS_LIBRARY_${LIB} ${LIB_PATH})
    ELSE (LIB_PATH)
      SET (GRASS_LIBRARY_${LIB} NOTFOUND)
      SET (GRASS_LIBRARIES_FOUND FALSE)
    ENDIF (LIB_PATH)

  ENDFOREACH (LIB)

  # LIB_PATH is only temporary variable, so hide it (is it possible to delete a variable?)
  MARK_AS_ADVANCED(LIB_PATH)

  IF (GRASS_INCLUDE_DIR AND GRASS_LIBRARIES_FOUND)
    SET (GRASS_FOUND TRUE)
    SET (GRASS_PREFIX ${G_PREFIX})
  ENDIF (GRASS_INCLUDE_DIR AND GRASS_LIBRARIES_FOUND)
    
  MARK_AS_ADVANCED ( GRASS_INCLUDE_DIR )

ENDMACRO (CHECK_GRASS)

###################################
# search for grass installations

# list of paths which to search - user's choice as first
SET (GRASS_PATHS ${GRASS_PREFIX} /usr/lib/grass c:/msys/local)

# mac-specific path
IF (APPLE)
  SET (GRASS_PATHS ${GRASS_PATHS}
    /Applications/GRASS-6.3.app/Contents/MacOS
    /Applications/GRASS.app/Contents/Resources
  )
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

   IF (NOT GRASS_FIND_QUIETLY)
      MESSAGE(STATUS "Found GRASS: ${GRASS_PREFIX} (${GRASS_VERSION})")
   ENDIF (NOT GRASS_FIND_QUIETLY)

ELSE (GRASS_FOUND)

   IF (WITH_GRASS)

     IF (GRASS_FIND_REQUIRED)
        MESSAGE(FATAL_ERROR "Could not find GRASS")
     ELSE (GRASS_FIND_REQUIRED)
        MESSAGE(STATUS "Could not find GRASS")
     ENDIF (GRASS_FIND_REQUIRED)

   ENDIF (WITH_GRASS)

ENDIF (GRASS_FOUND)
