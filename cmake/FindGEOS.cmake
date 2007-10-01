
# CMake module to search for GEOS library
#
# If it's found it sets GEOS_FOUND to TRUE
# and following variables are set:
#    GEOS_INCLUDE_DIR
#    GEOS_LIBRARY


FIND_PATH(GEOS_INCLUDE_DIR geos.h 
  /usr/local/include 
  /usr/include 
  c:/msys/local/include
  #MSVC
  C:/dev/cpp/geos-2.2.1/source/headers
  )

FIND_LIBRARY(GEOS_LIBRARY NAMES geos PATHS 
  /usr/local/lib 
  /usr/lib 
  c:/msys/local/lib
  #MSVC
  C:/dev/cpp/geos-2.2.1/source
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
