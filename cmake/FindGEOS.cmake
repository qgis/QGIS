
# CMake module to search for GEOS library
#
# If it's found it sets GEOS_FOUND to TRUE
# and following variables are set:
#    GEOS_INCLUDE_DIR
#    GEOS_LIBRARY


# Normally there is no need to specify /usr/... paths because 
# cmake will look there automatically. However the NO_DEFAULT_PATH
# prevents this behaviour allowing you to use no standard file
# locations in preference over standard ones. Note in this case
# you then need to explicitly add /usr and /usr/local prefixes
# to the search list. This applies both to FIND_PATH and FIND_LIBRARY
FIND_PATH(GEOS_INCLUDE_DIR geos.h 
  "$ENV{LIB_DIR}/include"
  /usr/local/include 
  /usr/include 
  #mingw
  c:/msys/local/include
  NO_DEFAULT_PATH
  )

FIND_LIBRARY(GEOS_LIBRARY NAMES geos PATHS 
  "$ENV{LIB_DIR}/lib"
  /usr/local/lib 
  /usr/lib 
  #mingw
  c:/msys/local/lib
  NO_DEFAULT_PATH
  )

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
