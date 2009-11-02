
# macro that checks for grass installation in specified directory

MACRO (CHECK_GRASS G_PREFIX)

  FIND_PATH (GRASS_INCLUDE_DIR grass/version.h ${G_PREFIX}/include)

  SET (GRASS_LIB_NAMES gis vect dig2 dbmiclient dbmibase shape dgl rtree datetime linkm form gproj)

  SET (GRASS_LIBRARIES "")

  FOREACH (LIB ${GRASS_LIB_NAMES})
    SET(LIB_PATH NOTFOUND)
    FIND_LIBRARY(LIB_PATH NAMES grass_${LIB} PATHS ${G_PREFIX}/lib NO_DEFAULT_PATH)

    IF (LIB_PATH)
      IF (NOT GRASS_LIBRARIES STREQUAL NOTFOUND)
        SET (GRASS_LIBRARIES ${GRASS_LIBRARIES} ${LIB_PATH})
      ENDIF (NOT GRASS_LIBRARIES STREQUAL NOTFOUND)
    ELSE (LIB_PATH)
      SET (GRASS_LIBRARIES NOTFOUND)
    ENDIF (LIB_PATH)

  ENDFOREACH (LIB)

  # LIB_PATH is only temporary variable, so hide it (is it possible to delete a variable?)
  MARK_AS_ADVANCED(LIB_PATH)

  IF (GRASS_INCLUDE_DIR AND GRASS_LIBRARIES)
    SET (GRASS_FOUND TRUE)
    SET (GRASS_PREFIX ${G_PREFIX})
  ENDIF (GRASS_INCLUDE_DIR AND GRASS_LIBRARIES)
    
  MARK_AS_ADVANCED (
    GRASS_INCLUDE_DIR
    GRASS_LIBRARIES
  )

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

  # openpty is currently needed for GRASS shell
  INCLUDE(CheckFunctionExists)
  IF (APPLE)
  SET (CMAKE_REQUIRED_INCLUDES util.h)
  ELSE (APPLE)
  SET (CMAKE_REQUIRED_INCLUDES pty.h)
  SET (CMAKE_REQUIRED_LIBRARIES util)
  ENDIF (APPLE)
  CHECK_FUNCTION_EXISTS(openpty HAVE_OPENPTY)

  # add 'util' library to the dependencies
  IF (HAVE_OPENPTY AND NOT APPLE)
    FIND_LIBRARY(OPENPTY_LIBRARY NAMES util PATHS /usr/local/lib /usr/lib c:/msys/local/lib)
    SET (GRASS_LIBRARIES ${GRASS_LIBRARIES} ${OPENPTY_LIBRARY})
  ENDIF (HAVE_OPENPTY AND NOT APPLE)

ELSE (GRASS_FOUND)

   IF (WITH_GRASS)

     IF (GRASS_FIND_REQUIRED)
        MESSAGE(FATAL_ERROR "Could not find GRASS")
     ELSE (GRASS_FIND_REQUIRED)
        MESSAGE(STATUS "Could not find GRASS")
     ENDIF (GRASS_FIND_REQUIRED)

   ENDIF (WITH_GRASS)

ENDIF (GRASS_FOUND)
