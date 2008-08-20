
# CMake module to search for GEOS library
#
# If it's found it sets GEOS_FOUND to TRUE
# and following variables are set:
#    GEOS_INCLUDE_DIR
#    GEOS_LIBRARY


# FIND_PATH and FIND_LIBRARY normally search standard locations
# before the specified paths. To search non-standard paths first,
# FIND_* is invoked first with specified paths and NO_DEFAULT_PATH
# and then again with no specified paths to search the default
# locations. When an earlier FIND_* succeeds, subsequent FIND_*s
# searching for the same item do nothing. 
FIND_PATH(GEOS_INCLUDE_DIR geos_c.h
  "$ENV{LIB_DIR}/include"
  #mingw
  c:/msys/local/include
  NO_DEFAULT_PATH
  )
FIND_PATH(GEOS_INCLUDE_DIR geos_c.h)

FIND_LIBRARY(GEOS_LIBRARY NAMES geos_c PATHS 
  "$ENV{LIB_DIR}/lib"
  #mingw
  c:/msys/local/lib
  NO_DEFAULT_PATH
  )
FIND_LIBRARY(GEOS_LIBRARY NAMES geos_c)

IF (GEOS_INCLUDE_DIR AND GEOS_LIBRARY)
   SET(GEOS_FOUND TRUE)
ENDIF (GEOS_INCLUDE_DIR AND GEOS_LIBRARY)


IF (GEOS_FOUND)

   IF (NOT GEOS_FIND_QUIETLY)
      MESSAGE(STATUS "Found GEOS: ${GEOS_LIBRARY}")
   ENDIF (NOT GEOS_FIND_QUIETLY)

ELSE (GEOS_FOUND)

   IF (GEOS_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "Could not find GEOS")
   ENDIF (GEOS_FIND_REQUIRED)

ENDIF (GEOS_FOUND)
